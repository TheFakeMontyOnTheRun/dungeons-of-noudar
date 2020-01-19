#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <functional>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>

#include <stdio.h>
#include <cmath>
#include <vector>
#include <array>

using std::vector;
using std::array;

#include "IFileLoaderDelegate.h"

#include "NativeBitmap.h"
#include "Logger.h"

#include "SoundClip.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"


#include "NoudarDungeonSnapshot.h"
#include "GameNativeAPI.h"

#include "IRenderer.h"
#include "NativeBitmap.h"
#include "CGame.h"
#include "Common.h"
#include "NoudarGLES2Bridge.h"
#include "SplatAnimation.h"

#include "ETextures.h"
#include "VBORegister.h"
#include "VBORenderingJob.h"
#include "CTile3DProperties.h"
#include "Camera.h"
#include "RenderingJobSnapshotAdapter.h"
#include "CLerp.h"
#include "DungeonGLES2Renderer.h"
#include "LightningStrategy.h"
#include "VisibilityStrategy.h"
#include "LoadPNG.h"
#include "CStorageItem.h"

std::shared_ptr<odb::DungeonGLES2Renderer> gles2Renderer = nullptr;

std::map<int, glm::vec2> mPositions;

bool hasActiveSplats;
odb::AnimationList animationList;
vector<std::shared_ptr<odb::SplatAnimation>> splatAnimation;
long animationTime = 0;

std::shared_ptr<Knights::CGame> game;
std::shared_ptr<odb::NoudarGLES2Bridge> render;
odb::NoudarDungeonSnapshot snapshot;
const int DEFAULT_HEALTH = 999;

vector<std::shared_ptr<odb::SoundEmitter>> soundEmitters;
std::shared_ptr<odb::SoundListener> mainListener;


void playSound(int soundNum);

vector<std::shared_ptr<odb::NativeBitmap>>
loadBitmapList(std::string filename, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {
    auto data = fileLoader->loadFileFromPath(filename);
    std::stringstream dataStream;

    dataStream << data;

    std::string buffer;

    vector<std::shared_ptr<odb::NativeBitmap>> toReturn;

    while (dataStream.good()) {
        std::getline(dataStream, buffer);

        toReturn.push_back(loadPNG(buffer, fileLoader));
    }

    return toReturn;
}

vector<vector<std::shared_ptr<odb::NativeBitmap>>>
loadTexturesForLevel(int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {

    std::stringstream roomName("");
    roomName << "tiles";
    roomName << levelNumber;
    roomName << ".lst";
    std::string tilesFilename = roomName.str();
    auto data = fileLoader->loadFileFromPath(tilesFilename);
    std::stringstream dataStream;

    dataStream << data;

    std::string buffer;

    vector<vector<std::shared_ptr<odb::NativeBitmap>>> tilesToLoad;

    while (dataStream.good()) {
        std::getline(dataStream, buffer);
        vector<std::shared_ptr<odb::NativeBitmap>> textures;


        if (!buffer.empty()) {
            textures.push_back(loadPNG(buffer, fileLoader));
        }

        tilesToLoad.push_back(textures);
    }

    return tilesToLoad;
}

odb::CTilePropertyMap
loadTileProperties(int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {
    std::stringstream roomName("");
    roomName << "tiles";
    roomName << levelNumber;
    roomName << ".prp";
    std::string filename = roomName.str();

    auto data = fileLoader->loadFileFromPath(filename);

    return odb::CTile3DProperties::parsePropertyList(data);
}


bool setupGraphics(int w, int h, std::string vertexShader, std::string fragmentShader,
                   std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {

    gles2Renderer = std::make_shared<odb::DungeonGLES2Renderer>();

    animationTime = 0;

    auto toReturn = gles2Renderer->init(w, h, vertexShader.c_str(), fragmentShader.c_str());

    auto textures = loadTexturesForLevel(0, fileLoader);

    gles2Renderer->setTexture(textures);
    gles2Renderer->reloadTextures();


    gles2Renderer->setTileProperties(
            loadTileProperties(game != nullptr ? game->getLevelNumber() : 0, fileLoader));

    return toReturn;
}

void renderFrame(long delta) {
    if (gles2Renderer != nullptr && game != nullptr) {

        gles2Renderer->updateFadeState(delta);
        auto cursor = game->getMap()->getTargetProjection(game->getMap()->getAvatar());

        snapshot.movingCharacters = animationList;
        snapshot.mTimestamp = animationTime;
        snapshot.mCursorPosition = cursor;
        snapshot.mCameraId = game->getCurrentActorId();
        snapshot.mCameraPosition = game->getMap()->getAvatar()->getPosition();
        gles2Renderer->render(snapshot);
        gles2Renderer->updateCamera(delta);
    }
}

void shutdown() {
    gles2Renderer->shutdown();
    animationList.clear();
    mPositions.clear();
    animationTime = 0;

    gles2Renderer = nullptr;
}

void updateAnimations(long delta) {
    auto it = animationList.begin();
    while (it != animationList.end()) {
        if (animationTime - (std::get<2>(it->second)) >= odb::kAnimationLength) {
            it = animationList.erase(it);
        } else {
            it = next(it);
        }
    }

    hasActiveSplats = splatAnimation.size() > 0;

    for (auto splat : splatAnimation) {
        splat->update(delta);
    }

    splatAnimation.erase(std::remove_if(splatAnimation.begin(), splatAnimation.end(),
                                        [](std::shared_ptr<odb::SplatAnimation> splat) { return splat->isFinished(); }
    ), splatAnimation.end());

    if (hasActiveSplats) {
        game->tick();
    }

    if (hasActiveSplats || animationList.size() > 0) {
        gles2Renderer->invalidateCachedBatches();
    }

    animationTime += delta;
}

void addCharacterMovement(int id, glm::vec2 previousPosition, glm::vec2 newPosition) {

    auto movement = std::make_tuple<>(previousPosition, newPosition, animationTime);

    if (animationList.count(id) > 0) {

        auto animation = animationList[id];
        auto prevPosition = std::get<0>(animation);
        animation = std::make_tuple<>(prevPosition, newPosition, animationTime);
    }

    animationList[id] = movement;

    auto floorType = snapshot.map[newPosition.y][newPosition.x];

    if (floorType == '.' || floorType == '-') {
        playSound(0);
    } else if (floorType == '_' || floorType == '=') {
        playSound(1);
    } else {
        if (floorType == '{' || floorType == '(' || floorType == ')' || floorType == '}' ||
            floorType == '2' || floorType == '7' || '~') {
            playSound(1);
        }
    }
}

void updateCharacterMovements(const odb::IntMap &idsLocal) {

    for (int y = 0; y < Knights::kMapSize; ++y) {
        for (int x = 0; x < Knights::kMapSize; ++x) {

            int id = idsLocal[y][x];
            snapshot.ids[y][x] = id;

            if (id != 0) {
                auto previousPosition = mPositions[id];

                if (previousPosition != glm::vec2(x, y)) {
                    mPositions[id] = glm::vec2(x, y);

                    if (game->getTurn() > 0) {
                        addCharacterMovement(id, previousPosition, mPositions[id]);
                    }
                }
            }
        }
    }
}

void startFadingIn() {
    if (gles2Renderer != nullptr) {
        gles2Renderer->startFadingIn();
    }
}

void startFadingOut() {
    if (gles2Renderer != nullptr) {
        gles2Renderer->startFadingOut();
    }
}

bool isAnimating() {

    if (gles2Renderer != nullptr) {
        return gles2Renderer->isAnimating() && !hasActiveSplats;
    }

    return false;
}

void rotateCameraLeft() {

    if (gles2Renderer != nullptr && !isAnimating()) {
        gles2Renderer->rotateLeft();
        render->setNextCommand('i');
        game->tick();
        render->setNextCommand('.');
    }
}

void rotateCameraRight() {

    if (gles2Renderer != nullptr && !isAnimating()) {
        gles2Renderer->rotateRight();
        render->setNextCommand('p');
        game->tick();
        render->setNextCommand('.');
    }
}

void setEyeViewMatrix(float *eyeView) {
    if (gles2Renderer != nullptr) {
        gles2Renderer->setEyeView(eyeView);
    }
}

void setPerspectiveMatrix(float *perspectiveMatrix) {
    if (gles2Renderer != nullptr) {
        gles2Renderer->setPerspectiveMatrix(perspectiveMatrix);
    }
}

void setAngleXZ(float XZAngle) {
    if (gles2Renderer != nullptr) {
        gles2Renderer->setAngleXZ(XZAngle);
    }
}

void setAngleYZ(float YZAngle) {
    if (gles2Renderer != nullptr) {
        gles2Renderer->setAngleYZ(YZAngle);
    }
}

void readMap(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate) {
    gles2Renderer->shutdown();
    render = std::make_shared<odb::NoudarGLES2Bridge>();

    auto onMonsterDead = [&](Knights::Vec2i pos) {
        playSound(6);
    };

    auto onPlayerDead = [&](Knights::Vec2i pos) {
        playSound(7);
        game->playLevel(8);
    };

    auto onPlayerAttack = [&](Knights::Vec2i pos) {
        playSound(5);
    };

    auto onMonsterAttack = [&](Knights::Vec2i pos) {

    };

    auto onMonsterDamaged = [&](Knights::Vec2i pos) {
        auto splat = std::make_shared<odb::SplatAnimation>(pos);
        splatAnimation.push_back(splat);
        splat->startSplatAnimation();
        playSound(3);
    };

    auto onProjectileHit = [&](Knights::Vec2i pos) {
        auto splat = std::make_shared<odb::SplatAnimation>(pos);
        splatAnimation.push_back(splat);
        splat->startSplatAnimation();
    };


    auto onPlayerDamaged = [&](Knights::Vec2i pos) {
        playSound(4);
    };


    auto onLevelLoaded = [fileLoaderDelegate]() {

        auto textures = loadTexturesForLevel(game != nullptr ? game->getLevelNumber() : 0,
                                             fileLoaderDelegate);

        forceDirection(0);
        render->reset();
        animationList.clear();
        splatAnimation.clear();
        animationTime = 0;
        hasActiveSplats = false;

        if (gles2Renderer != nullptr) {
            gles2Renderer->setTexture(textures);
            gles2Renderer->reloadTextures();
            gles2Renderer->setTileProperties(
                    loadTileProperties(game != nullptr ? game->getLevelNumber() : 0,
                                       fileLoaderDelegate));
            gles2Renderer->resetCamera();
        }

        if (game != nullptr) {
            game->tick();
        }
    };

    auto gameDelegate = std::make_shared<Knights::CGameDelegate>();

    gameDelegate->setMonsterAttackedCallback(onMonsterAttack);
    gameDelegate->setMonsterDiedCallback(onMonsterDead);
    gameDelegate->setMonsterDamagedCallback(onMonsterDamaged);
    gameDelegate->setPlayerAttackedCallback(onPlayerAttack);
    gameDelegate->setPlayerDiedCallback(onPlayerDead);
    gameDelegate->setPlayerDamagedCallback(onPlayerDamaged);
    gameDelegate->setOnLevelLoadedCallback(onLevelLoaded);
    gameDelegate->setProjectileCallback(onProjectileHit);

    game = std::make_shared<Knights::CGame>(fileLoaderDelegate, render, gameDelegate);

    if (game != nullptr) {
        game->tick();
    }
}

int soundToPlay = -1;

void playSound(int soundNum) {
#ifndef ANDROID
    if (!soundEmitters.empty()) {
        soundEmitters[soundNum]->play(mainListener);
    }
#else
	if ( soundToPlay < soundNum ) {
    	soundToPlay = soundNum;
    }
#endif
}

void moveUp() {

    if (game != nullptr && !isAnimating()) {
        render->setNextCommand(Knights::kMovePlayerForwardCommand);
        game->tick();
        render->setNextCommand('.');
    }
}

void moveDown() {

    if (game != nullptr && !isAnimating()) {
        render->setNextCommand(Knights::kMovePlayerBackwardCommand);
        game->tick();
        render->setNextCommand('.');
    }
}

void moveLeft() {

    if (game != nullptr && !isAnimating()) {
        render->setNextCommand(Knights::kStrafeLeftCommand);
        game->tick();
        render->setNextCommand('.');
    }
}

void moveRight() {

    if (game != nullptr && !isAnimating()) {
        render->setNextCommand(Knights::kStrafeRightCommand);
        game->tick();
        render->setNextCommand('.');
    }
}

void gameLoopTick(long ms) {
    updateAnimations(ms);
}

void setSoundEmitters(vector<std::shared_ptr<odb::SoundEmitter>> emitters,
                      std::shared_ptr<odb::SoundListener> listener) {
    soundEmitters = emitters;
    mainListener = listener;
}

void forceDirection(int direction) {

    if (game == nullptr || render == nullptr) {
        return;
    }

    char directions[] = {'r', 'f', 'c', 'd'};
    render->setNextCommand(directions[direction]);
    game->tick();
    render->setNextCommand('.');
}


void setSnapshot(const odb::NoudarDungeonSnapshot &newSnapshot) {

    hasActiveSplats = false;

    snapshot = newSnapshot;

    for (int y = 0; y < Knights::kMapSize; ++y) {
        for (int x = 0; x < Knights::kMapSize; ++x) {
            snapshot.splat[y][x] = -1;
        }
    }

    for (auto &splatAnim : splatAnimation) {
        auto pos = splatAnim->getPosition();
        snapshot.splat[pos.y][pos.x] = static_cast<odb::ETextures >( splatAnim->getSplatFrame());
    }

    updateCharacterMovements(snapshot.ids);
}

void interact() {
    if (game != nullptr) {
        render->setNextCommand('\t');
        game->tick();
        render->setNextCommand('.');
    }
}

void pickupItem() {
    if (game != nullptr) {
        render->setNextCommand('[');
        game->tick();
        render->setNextCommand('.');
    }
}

void cycleNextItem() {
    if (game != nullptr) {
        render->setNextCommand('-');
        game->tick();
        render->setNextCommand('.');
    }
}

void cyclePrevItem() {
    if (game != nullptr) {
        render->setNextCommand('=');
        game->tick();
        render->setNextCommand('.');

        auto item = game->getMap()->getAvatar()->getSelectedItem();
    }
}

bool isThereAnyObjectInFrontOfYou() {
    auto map = game->getMap();
    auto actor = map->getAvatar();

    return map->getItemAt(map->getActorTargetPosition(actor)) != nullptr;
}

std::string getCurrentObjectName() {
    auto map = game->getMap();
    auto actor = map->getAvatar();
    auto item = actor->getSelectedItem();
    if (item != nullptr) {
        return item->to_string();
    } else {
        return "";
    }
}

int lastHP = 100;

int getHP() {

    if (game == nullptr) {
        return DEFAULT_HEALTH;
    }

    auto map = game->getMap();

    if (map == nullptr) {
        return DEFAULT_HEALTH;
    }

    auto actor = map->getAvatar();

    if (actor == nullptr) {
        return DEFAULT_HEALTH;
    }

    lastHP = actor->getHP();

    return lastHP;
}

//HAS TO BE CALLED BEFORE GET FAITH!
int getTint() {
    if (game == nullptr || game->getMap() == nullptr || game->getMap()->getAvatar() == nullptr ) {
        return 0;
    }

    if (game->getMap()->getAvatar()->getHP() < lastHP ) {
        return -1;
    } else if (game->getMap()->getAvatar()->getHP() > lastHP ) {
        return 1;
    } else {
        return 0;
    }
}


Knights::EDirection getCurrentDirection() {
    auto map = game->getMap();
    auto actor = map->getAvatar();

    return actor->getDirection();
}


bool isPlaying() {
    return getHP() > 0;
}

int getLevel() {
    if (game == nullptr) {
        return 0;
    }

    return game->getLevelNumber();
}

char getCurrentItemSymbol() {
	if (game == nullptr || game->getMap() == nullptr || game->getMap()->getAvatar() == nullptr ) {
		return '.';
	}

	return game->getMap()->getAvatar()->getSelectedItem()->getView();
}

int hasItem(char item ) {
    if (game == nullptr || game->getMap() == nullptr || game->getMap()->getAvatar() == nullptr ) {
        return 0;
    }

    std::shared_ptr<Knights::CItem> itemPtr = game->getMap()->getAvatar()->getItemWithSymbol(item);

    return !!itemPtr;
}

void setCurrentItem( char item ) {
    if (game == nullptr || game->getMap() == nullptr || game->getMap()->getAvatar() == nullptr ) {
        return;
    }

    game->getMap()->getAvatar()->suggestCurrentItem(item);
}


int getItemAvailability(char item ) {
	if (game == nullptr || game->getMap() == nullptr || game->getMap()->getAvatar() == nullptr ) {
		return 0;
	}

	std::shared_ptr<Knights::CItem> itemPtr = game->getMap()->getAvatar()->getItemWithSymbol(item);
    auto storageItem = (Knights::CStorageItem*)itemPtr.get();
	if (storageItem != nullptr) {
		return storageItem->getAmount();
	} else {
		return -1;
	}
}
