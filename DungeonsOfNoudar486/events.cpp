//general stuff below
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <memory>
#include <utility>
#include <string>
#include <map>
#include <algorithm>
#include <chrono>
#include <sg14/fixed_point>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;
using namespace std::chrono;
using sg14::fixed_point;

#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "IFileLoaderDelegate.h"
#include "CGame.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "commands/IGameCommand.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"
#include "LoadPNG.h"

enum class EScreenState {
    kIntro, kLoading, kChapter, kGame, kGameOver, kVictory, kExit
};

enum class ESoundDriver {
    kNone, kPcSpeaker, kOpl2Lpt, kAdlib, kTandy, kCovox
};

EScreenState screenState = EScreenState::kIntro;
extern ESoundDriver soundDriver;

std::shared_ptr<odb::NativeBitmap> stateBG = nullptr;
std::string stateText = "";
std::string stateTransitionText = "";
uint8_t bgColour;
uint8_t textStartingLine;
std::shared_ptr<odb::CPackedFileReader> fileLoader = nullptr;

void setScreenState( EScreenState newState, std::shared_ptr<Knights::CGame> game ) {

    switch( newState ) {
        case EScreenState::kIntro:
            stateBG = loadPNG("logo.png", fileLoader);
            stateText = fileLoader->loadFileFromPath("title.txt");
            stateTransitionText = "Press Enter to continue";
            bgColour = 0;
            textStartingLine = 9;
            break;
        case EScreenState::kLoading:
            stateBG = loadPNG("intro.png", fileLoader);
            stateText = "";
            stateTransitionText = "Loading...";
            bgColour = 0;
            textStartingLine = 9;
            break;
        case EScreenState::kChapter:
            char buffer[41];
            snprintf(buffer, 39, "chapter%d.txt", game->getLevelNumber());
            stateBG = loadPNG("intro.png", fileLoader);
            stateText = fileLoader->loadFileFromPath(buffer);;
            stateTransitionText = "Press Enter to continue";
            bgColour = 0;
            textStartingLine = 9;
            break;
        case EScreenState::kGame:
            break;
        case EScreenState::kVictory:
            stateBG = loadPNG("finis_gloriae_mundi.png", fileLoader);
            stateText = fileLoader->loadFileFromPath("chapter7.txt");
            stateTransitionText = "                    Press enter to exit";
            bgColour = odb::renderer->getPaletteEntry(0xFFFFFFFF);
            textStartingLine = 2;
            break;
        case EScreenState::kGameOver:
            stateBG = loadPNG("in_ictu_oculi.png", fileLoader);
            stateText = fileLoader->loadFileFromPath("gameover.txt");
            stateTransitionText = "                    Press enter to exit";
            bgColour = odb::renderer->getPaletteEntry(0xFFFFFFFF);
            textStartingLine = 2;
            break;
        case EScreenState::kExit:
            game->setIsPlaying(false);
            break;
    }

    screenState = newState;
}

void onContinuePressed( std::shared_ptr<Knights::CGame> game) {
    switch( screenState ) {
        case EScreenState::kIntro:
            game->playLevel(0);
            break;
        case EScreenState::kLoading:
            setScreenState( EScreenState::kChapter, game);
            break;
        case EScreenState::kChapter:
            setScreenState( EScreenState::kGame, game);
            break;
        case EScreenState::kGame:
            break;
        case EScreenState::kVictory:
            setScreenState( EScreenState::kExit, game);
            break;
        case EScreenState::kGameOver:
            setScreenState( EScreenState::kExit, game);
            break;
        case EScreenState::kExit:
#ifdef __EMSCRIPTEN__
            setScreenState( EScreenState::kIntro, game);
#else
            game->setIsPlaying(false);
#endif
            break;
    }
}

void initOPL2(int port);

void playTune(const std::string &);

void setupOPL2(int port);

void stopSounds();

void soundTick();

void muteSound();

void playMusic(const std::string &musicTrack);

namespace odb {
    std::shared_ptr<CRenderer> renderer = nullptr;
}


void initOPL2(int port = -1) {
    setupOPL2(port);
}

void *operator new[](size_t size, const char *pName, int flags, unsigned debugFlags,
                     const char *file, int line) {
    return new uint8_t[size];
}

void *operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char *pName,
                     int flags, unsigned debugFlags, const char *file, int line) {
    return new uint8_t[size];;
}



void showText(std::shared_ptr<odb::NativeBitmap> bg, const std::string &mainText, const std::string &bottom,
              uint8_t bgSolidColour, uint8_t startingLine) {


    odb::renderer->fill(0, 0, 320, 200, bgSolidColour);
    odb::renderer->drawBitmap(0, 0, bg);

    uint8_t mainTextColour = odb::renderer->getPaletteEntry(0xFFFF0000);
    uint8_t bottomTextColour = odb::renderer->getPaletteEntry(0xFF888888);
    odb::renderer->drawTextAt(1, startingLine, mainText.c_str(), mainTextColour);
    odb::renderer->drawTextAt(1, 25, bottom.c_str(), bottomTextColour);
    odb::renderer->flip();
}

void renderTick(long ms) {

    switch (screenState ) {
        case EScreenState::kGame:
            odb::renderer->render(ms);
            break;
        default:
            showText(stateBG, stateText, stateTransitionText, bgColour, textStartingLine );
            break;
    }

    odb::renderer->handleSystemEvents();
}

std::shared_ptr<Knights::CGame> game;

void playerIsDead() {
    setScreenState(EScreenState::kGameOver, game);
}

void handleConsoleLines(Knights::CommandType command, int playerHealthDiff, int targetHealthDiff,
                        std::shared_ptr<odb::CRenderer> renderer, std::shared_ptr<Knights::CActor> actorAtTarget) {

    uint8_t playerDealtDamageColour = renderer->getPaletteEntry(0xFFFF0000);
    uint8_t playerTookDamageColour = renderer->getPaletteEntry(0xFF0000FF);
    uint8_t playerGainedFaithColour = renderer->getPaletteEntry(0xFF00FF00);
    uint8_t commandColour = renderer->getPaletteEntry(0xFFFFFFFF);
    uint8_t effectColour = renderer->getPaletteEntry(0xFF888888);

    char *soundToPlay = nullptr;

    if (command != Knights::kNullCommand) {
        char buffer[41];
        snprintf(&buffer[0], 39, "%s", game->getLastCommand().c_str());
        renderer->appendToLog(buffer, commandColour);

        auto say = game->getMap()->getAvatar()->getCurrentSay();
        if (!say.empty()) {
            snprintf(&buffer[0], 39, "%s", say.c_str());
            renderer->appendToLog(buffer, effectColour);
            game->getMap()->getAvatar()->setCurrentSay("");
        }
    }

    if (command == Knights::kUseCurrentItemInInventoryCommand) {
        playTune("i114t1o8f");
    }

    if (command == Knights::kCycleRightInventoryCommand) {
        playTune("t200i98a");
    }

    if (command == Knights::kPickItemCommand) {
        playTune("t200i9o1fa");
    }

    if (targetHealthDiff < 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player dealt %d on %s", targetHealthDiff, actorAtTarget->getName().c_str());
        renderer->appendToLog(buffer, playerDealtDamageColour);
        if (actorAtTarget == nullptr || !actorAtTarget->isAlive()) {
            renderer->addDeathAt(actorAtTarget->getPosition());
            playTune("t200i101o8ao4ao2ao1a");
        } else {
            renderer->addSplatAt(actorAtTarget->getPosition());
            playTune("t200i101o3afo1a");
        }
    }

    if (playerHealthDiff < 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player took %d of damage", std::abs(playerHealthDiff));
        renderer->appendToLog(buffer, playerTookDamageColour);
        renderer->startDamageHighlight();
        playTune("t200i53o3fo1f");
    } else if (playerHealthDiff > 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player gained %d of faith", std::abs(playerHealthDiff));
        renderer->appendToLog(buffer, playerGainedFaithColour);
        renderer->startHealHighlight();
        playTune("t200i52o4defg");
    }
}

void loopTick(long diff) {

    int healthAtTargetBefore = 0;
    int healthAtTargetAfter = 0;

    auto playerHealthBefore = game->getMap()->getAvatar()->getHP();
    auto cursorPosition = game->getMap()->getTargetProjection(game->getMap()->getAvatar());
    auto actorAtTarget = game->getMap()->getActorAt(cursorPosition);

    if (actorAtTarget != nullptr) {
        healthAtTargetBefore = actorAtTarget->getHP();
    } else {
        healthAtTargetBefore = 0;
    }

#ifdef __EMSCRIPTEN__
    diff = 50;
#endif
    renderTick(diff);
    Knights::CommandType command = odb::renderer->peekInput();

    if ( command == Knights::kStartCommand ) {

        odb::renderer->mBufferedCommand = Knights::kNullCommand;
        onContinuePressed(game);
        return;
    }

    game->tick();
    soundTick();

    if (actorAtTarget != nullptr) {
        healthAtTargetAfter = actorAtTarget->getHP();
    } else {
        healthAtTargetAfter = 0;
    }

    auto targetHealthDiff = healthAtTargetAfter - healthAtTargetBefore;
    auto playerHealthAfter = game->getMap()->getAvatar()->getHP();
    auto playerHealthDiff = playerHealthAfter - playerHealthBefore;

    handleConsoleLines(command, playerHealthDiff, targetHealthDiff, odb::renderer, actorAtTarget);



    if (!game->getMap()->getAvatar()->isAlive()) {
        playerIsDead();
    }
}

void init() {
    const auto LEVEL_LIMIT = 7;

    auto delegate = std::make_shared<Knights::CGameDelegate>();
    fileLoader = std::make_shared<odb::CPackedFileReader>("data.pfs");
    odb::renderer = std::make_shared<odb::CRenderer>(fileLoader);
    game = std::make_shared<Knights::CGame>(fileLoader, odb::renderer, delegate);

    auto onLevelWillLoad = [&]() {
        setScreenState(EScreenState::kLoading, game);
        renderTick(50);
    };

    delegate->setOnLevelWillLoadCallback(onLevelWillLoad);

    auto onLevelLoaded = [&]() {


        if (game->getLevelNumber() >= LEVEL_LIMIT) {
                setScreenState(EScreenState::kVictory, game);
        } else  {
            auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
            odb::renderer->loadTextures(odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader),
                                        tileProperties);
            setScreenState(EScreenState::kChapter, game);
        }
    };
    delegate->setOnLevelLoadedCallback(onLevelLoaded);

    setScreenState(EScreenState::kIntro, game);
}

void shutdown() {
    if (soundDriver != ESoundDriver::kNone) {
        stopSounds();
    }
}

bool isPlaying() {
    return game->isPlaying();
}

void pushCommand(char cmd ) {

    if ( cmd == Knights::kStartCommand ) {

        odb::renderer->mBufferedCommand = Knights::kNullCommand;
        onContinuePressed(game);
        return;
    }

    odb::renderer->mBufferedCommand = cmd;
    odb::renderer->mCached = false;
    loopTick(1);
}

uint8_t *getFramebuffer() {
    return odb::renderer->getBufferData();
}

uint32_t *getPalette() {
    return odb::renderer->mPalette.data();
}
