#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <map>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <array>
#include <stdio.h>
#include <cmath>

#include "IFileLoaderDelegate.h"

#include "NativeBitmap.h"
#include "Texture.h"
#include "Material.h"
#include "Trig.h"
#include "TrigBatch.h"
#include "MeshObject.h"
#include "MaterialList.h"
#include "Scene.h"
#include "WavefrontOBJReader.h"
#include "Logger.h"

#include "AudioSink.h"
#include "SoundClip.h"
#include "SoundUtils.h"
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
#include "RenderingJobSnapshotAdapter.h"
#include "Camera.h"
#include "CLerp.h"
#include "DungeonGLES2Renderer.h"
#include "LightningStrategy.h"
#include "VisibilityStrategy.h"
#include "LoadPNG.h"


#if defined(__ANDROID__ ) || defined(__EMSCRIPTEN__) || defined(MESA_GLES2) || defined(TARGET_IOS)
#include "OverlayNanoVGRenderer.h"
std::shared_ptr<odb::OverlayNanoVGRenderer> overlayRenderer = nullptr;
#endif

std::shared_ptr<odb::DungeonGLES2Renderer> gles2Renderer = nullptr;

std::map<int, glm::vec2> mPositions;

bool hasActiveSplats;
odb::AnimationList animationList;
long animationTime = 0;
std::vector<std::shared_ptr<odb::NativeBitmap>> textures;
std::shared_ptr<Knights::CGame> game;
std::shared_ptr<odb::NoudarGLES2Bridge> render;
std::vector<std::shared_ptr<odb::SoundEmitter>> soundEmitters;
std::vector<std::shared_ptr<odb::SplatAnimation>> splatAnimation;
std::shared_ptr<odb::SoundListener> mainListener;
odb::CTilePropertyMap tileProperties;
odb::NoudarDungeonSnapshot snapshot;
std::vector< std::shared_ptr<odb::Scene>> loadedMeshes;

bool setupGraphics(int w, int h, std::string vertexShader, std::string fragmentShader, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {

	gles2Renderer = std::make_shared<odb::DungeonGLES2Renderer>();

	textures = {
			loadPNG("grass.png", fileLoader),
			loadPNG("stonefloor.png", fileLoader),
			loadPNG("bricks.png", fileLoader),
			loadPNG("arch.png", fileLoader),
			loadPNG("bars.png", fileLoader),
			loadPNG("begin.png", fileLoader),
			loadPNG("exit.png", fileLoader),
			loadPNG("bricks_blood.png", fileLoader),
			loadPNG("bricks_candles.png", fileLoader),
			loadPNG("foe0.png", fileLoader),
			loadPNG("foe1.png", fileLoader),
			loadPNG("foe2.png", fileLoader),
			loadPNG("foe3.png", fileLoader),
			loadPNG("foe4.png", fileLoader),
			loadPNG("foe5.png", fileLoader),
			loadPNG("crusader0.png", fileLoader),
			loadPNG("crusader1.png", fileLoader),
			loadPNG("crusader2.png", fileLoader),
			loadPNG("shadow.png", fileLoader),
			loadPNG("ceiling.png", fileLoader),
			loadPNG("ceilingdoor.png", fileLoader),
			loadPNG("ceilingbegin.png", fileLoader),
			loadPNG("ceilingend.png", fileLoader),
			loadPNG("splat0.png", fileLoader),
			loadPNG("splat1.png", fileLoader),
			loadPNG("splat2.png", fileLoader),
			loadPNG("ceilingbars.png", fileLoader),
			loadPNG("clouds.png", fileLoader),
			loadPNG("stonegrassfar.png", fileLoader),
			loadPNG("grassstonefar.png", fileLoader),
			loadPNG("stonegrassnear.png", fileLoader),
			loadPNG("grassstonenear.png", fileLoader),
			loadPNG("cross.png", fileLoader),
	};

#if defined(__ANDROID__ ) || defined(__EMSCRIPTEN__) || defined(MESA_GLES2) || defined(TARGET_IOS)

	if ( overlayRenderer == nullptr ) {
		auto bitmaps = std::vector< std::shared_ptr<odb::NativeBitmap> > {
				loadPNG( "hand0.png", fileLoader  ),
				loadPNG( "hand1.png", fileLoader ),
				loadPNG( "bow2.png", fileLoader ),
		};

		overlayRenderer = std::make_shared<odb::OverlayNanoVGRenderer>(bitmaps);
	}

	overlayRenderer->setFrame( w, h );
    #endif

	gles2Renderer->setTexture(textures);
	animationTime = 0;

	auto toReturn = gles2Renderer->init(w, h, vertexShader.c_str(), fragmentShader.c_str());

	gles2Renderer->setTileProperties( tileProperties );

	for  ( const auto& mesh : loadedMeshes ) {
		gles2Renderer->setMesh( mesh );
	}

	loadedMeshes.clear();
	return toReturn;
}

void renderFrame(long delta) {
	if (gles2Renderer != nullptr && game != nullptr && textures.size() > 0) {

		gles2Renderer->updateFadeState(delta);
		auto cursor = game->getCursorPosition();

		snapshot.movingCharacters = animationList;
		snapshot.mTimestamp = animationTime;
		snapshot.mCursorPosition = cursor;
		snapshot.mCameraId = game->getCurrentActorId();

		gles2Renderer->render(snapshot);
		gles2Renderer->updateCamera(delta);
}

#if defined(__ANDROID__ ) || defined(__EMSCRIPTEN__) || defined(MESA_GLES2) || defined(TARGET_IOS)

	if ( overlayRenderer != nullptr ) {
		overlayRenderer->render(snapshot);
	}
#endif
}

void shutdown() {
	gles2Renderer->shutdown();
	animationList.clear();
	mPositions.clear();
	animationTime = 0;
	textures.clear();

	gles2Renderer = nullptr;

#if defined(__ANDROID__ ) || defined(__EMSCRIPTEN__) || defined(MESA_GLES2) || defined(TARGET_IOS)
	overlayRenderer = nullptr;
#endif
}

void updateAnimations( long delta ) {
	auto it = animationList.begin();
	while (it != animationList.end()) {
		if (animationTime - (std::get<2>(it->second)) >= odb::kAnimationLength) {
			it = animationList.erase(it);
		} else {
			it = next(it);
		}
	}

	hasActiveSplats = splatAnimation.size() > 0;

	for ( auto splat : splatAnimation ) {
		splat->update( delta );
	}

	splatAnimation.erase( std::remove_if( splatAnimation.begin(), splatAnimation.end(),
	                              [](std::shared_ptr<odb::SplatAnimation> splat){ return splat->isFinished();}
	), splatAnimation.end() );

	if ( hasActiveSplats ) {
		game->tick();
	}

	if ( hasActiveSplats || animationList.size() > 0 ) {
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

	auto floorType = snapshot.map[ newPosition.y ][ newPosition.x ];

	if ( floorType  == '.' || floorType == '-' ) {
		soundEmitters[0]->play(mainListener);
	} else if ( floorType == '_' || floorType == '=') {
		soundEmitters[1]->play(mainListener);
	} else {
		if ( floorType == '{' || floorType == '(' || floorType == ')' || floorType == '}' || floorType == '2' || floorType == '7' || '~' ) {
			soundEmitters[1]->play(mainListener);
		}
	}
}

void updateCharacterMovements(const odb::IntMap& idsLocal) {

	for (int y = 0; y < Knights::kMapSize; ++y) {
		for (int x = 0; x < Knights::kMapSize; ++x) {

			int id = idsLocal[y][x];
			snapshot.ids[y][x] = id;

			if (id != 0) {
				auto previousPosition = mPositions[id];

				if (previousPosition != glm::vec2(x, y) ) {
					mPositions[id] = glm::vec2(x, y);

					if ( game->getTurn() > 0  ) {
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

	if (gles2Renderer != nullptr&& !isAnimating() ) {
		gles2Renderer->rotateLeft();
		render->setNextCommand('i');
		game->tick();
		render->setNextCommand('.');
	}
}

void rotateCameraRight() {

	if (gles2Renderer != nullptr&& !isAnimating() ) {
		gles2Renderer->rotateRight();
		render->setNextCommand('p');
		game->tick();
		render->setNextCommand('.');
	}
}

void setEyeViewMatrix( float* eyeView ) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setEyeView(eyeView);
	}
}

void setPerspectiveMatrix( float* perspectiveMatrix ) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setPerspectiveMatrix(perspectiveMatrix);
	}
}


void setAngleXZ( float XZAngle ) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setAngleXZ(XZAngle);
	}
}

void setAngleYZ( float YZAngle ) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setAngleYZ(YZAngle);
	}
}

void loadMapData( std::string geoFile, std::string petFile ) {
}

void readMap( std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate, std::string tilePropertiesFile ) {

#if defined(__ANDROID__ ) || defined(__EMSCRIPTEN__) || defined(MESA_GLES2) || defined(TARGET_IOS)
	if ( overlayRenderer == nullptr ) {

		auto bitmaps = std::vector< std::shared_ptr<odb::NativeBitmap> > {
				loadPNG( "hand0.png", fileLoaderDelegate ),
				loadPNG( "hand1.png", fileLoaderDelegate ),
				loadPNG( "bow2.png", fileLoaderDelegate ),
		};

		overlayRenderer = std::make_shared<odb::OverlayNanoVGRenderer>( bitmaps );
	}

	overlayRenderer->loadFonts( fileLoaderDelegate );
#endif

	render = std::make_shared<odb::NoudarGLES2Bridge>();

	auto onMonsterDead = [&]( Knights::Vec2i pos ) {
		soundEmitters[ 4 ]->play( mainListener );
	};

	auto onPlayerDead = [&](Knights::Vec2i pos) {
		soundEmitters[ 6 ]->play( mainListener );
	};

	auto onPlayerAttack = [&](Knights::Vec2i pos) {
		soundEmitters[ 7 ]->play( mainListener );
	};

	auto onMonsterAttack = [&](Knights::Vec2i pos) {

	};

	auto onMonsterDamaged = [&](Knights::Vec2i pos) {
		auto splat = std::make_shared<odb::SplatAnimation>( pos );
		splatAnimation.push_back( splat );
		splat->startSplatAnimation();

		soundEmitters[ 3 ]->play( mainListener );
	};

	auto onPlayerDamaged = [&](Knights::Vec2i pos) {
		soundEmitters[ 5 ]->play( mainListener );
	};

	auto onLevelLoaded = [&]() {
	    forceDirection( 0 );

        if ( gles2Renderer != nullptr ) {
			gles2Renderer->resetCamera();
		}
	};

	auto gameDelegate = std::make_shared<Knights::CGameDelegate>();

	gameDelegate->setMonsterAttackedCallback( onMonsterAttack );
	gameDelegate->setMonsterDiedCallback( onMonsterDead );
	gameDelegate->setMonsterDamagedCallback(onMonsterDamaged);
	gameDelegate->setPlayerAttackedCallback( onPlayerAttack );
	gameDelegate->setPlayerDiedCallback( onPlayerDead );
	gameDelegate->setPlayerDamagedCallback( onPlayerDamaged );
	gameDelegate->setOnLevelLoadedCallback( onLevelLoaded );

	game = std::make_shared<Knights::CGame>( fileLoaderDelegate, render, gameDelegate );

	auto tilesData = fileLoaderDelegate->loadFileFromPath(tilePropertiesFile);

	setTileProperties( tilesData );

	if ( game != nullptr ) {
		game->tick();
	}
}

void moveUp() {

	if ( game != nullptr && !isAnimating() ) {
		render->setNextCommand('o');
		game->tick();
		render->setNextCommand('.');
	}
}

void moveDown() {

	if ( game != nullptr && !isAnimating() ) {
		render->setNextCommand('p');
		game->tick();
		render->setNextCommand('p');
		game->tick();
		render->setNextCommand('o');
		game->tick();
		render->setNextCommand('i');
		game->tick();
		render->setNextCommand('i');
		game->tick();
		render->setNextCommand('.');
	}
}

void moveLeft() {

	if ( game != nullptr && !isAnimating() ) {
		render->setNextCommand('k');
		game->tick();
		render->setNextCommand('.');
	}
}

void moveRight() {

	if ( game != nullptr && !isAnimating() ) {
		render->setNextCommand('l');
		game->tick();
		render->setNextCommand('.');
	}
}

void gameLoopTick( long ms ) {
    updateAnimations( ms );
}

void setSoundEmitters( std::vector<std::shared_ptr<odb::SoundEmitter>> emitters, std::shared_ptr<odb::SoundListener> listener ) {
	soundEmitters = emitters;
	mainListener = listener;
}

void forceDirection( int direction ) {

	if ( game == nullptr || render == nullptr ) {
		return;
	}

	char directions[] = { 'r', 'f', 'c', 'd'};
	render->setNextCommand( directions[ direction ] );
	game->tick();
	render->setNextCommand('.');
}


void setSnapshot(const odb::NoudarDungeonSnapshot& newSnapshot ) {

    hasActiveSplats = false;

    snapshot = newSnapshot;

    for (int y = 0; y < Knights::kMapSize; ++y) {
        for (int x = 0; x < Knights::kMapSize; ++x) {
            snapshot.splat[y][x] = -1;
        }
    }

    for ( auto& splatAnim : splatAnimation ) {
        auto pos = splatAnim->getPosition();
        snapshot.splat[pos.y][pos.x] = static_cast<odb::ETextures >( splatAnim->getSplatFrame());
    }

	updateCharacterMovements( snapshot.ids );
}

void setTileProperties( std::string tilePropertiesData ) {
	tileProperties = odb::CTile3DProperties::parsePropertyList( tilePropertiesData );
}

void loadMeshList( std::vector< std::string> meshes, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoaderDelegate ) {
	for ( const auto& mesh : meshes ) {

		std::istringstream meshStream( fileLoaderDelegate->loadFileFromPath( mesh));
		loadedMeshes.emplace_back( readScene( meshStream, fileLoaderDelegate ) );
	}
}

void interact() {
	if (game != nullptr) {
		render->setNextCommand('\t');
		game->tick();
		render->setNextCommand('.');
        overlayRenderer->playAnimation( animationTime, "crossbow-reload" );
	}
}

void pickupItem() {
    if (game != nullptr) {
        render->setNextCommand('[');
        game->tick();
        render->setNextCommand('.');
    }
}

void dropItem() {
    if (game != nullptr) {
        render->setNextCommand(']');
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
    }
}
