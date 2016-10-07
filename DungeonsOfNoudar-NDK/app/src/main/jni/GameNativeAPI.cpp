#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>
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
#include <mutex>

#include "gles2-renderer/NativeBitmap.h"
#include "gles2-renderer/Texture.h"
#include "gles2-renderer/Material.h"
#include "gles2-renderer/Trig.h"
#include "gles2-renderer/TrigBatch.h"
#include "gles2-renderer/MeshObject.h"
#include "gles2-renderer/MaterialList.h"
#include "gles2-renderer/Scene.h"
#include "gles2-renderer/WavefrontOBJReader.h"
#include "gles2-renderer/Logger.h"

#include "gles2-renderer/PETEntry.h"
#include "gles2-renderer/PETTable.h"
#include "gles2-renderer/GEOMap.h"

#include "DungeonGLES2Renderer.h"
#include "LightningStrategy.h"
#include "GameNativeAPI.h"

#include "Vec2i.h"
#include "IMapElement.h"
#include "CActor.h"
#include "CMap.h"
#include "IRenderer.h"
#include "CFalconKnight.h"
#include "CBullKnight.h"
#include "CTurtleKnight.h"
#include "CGame.h"

#include "NoudarGLES2Bridge.h"

std::string gVertexShader;
std::string gFragmentShader;

//std::string wavefrontCubeMesh;
//std::string wavefrontCubeMaterials;

std::shared_ptr<odb::DungeonGLES2Renderer> gles2Renderer = nullptr;
std::map<int, glm::vec2> mPositions;


bool hasActiveSplats;
odb::IntGameMap map;
odb::IntGameMap snapshot;
odb::IntGameMap splat;
odb::IntField ids;
odb::LightMap lightMap;
odb::AnimationList animationList;
long animationTime = 0;
std::mutex mutex;
bool hasCache = false;
odb::LightMap lightMapCache;
//std::shared_ptr<odb::Scene> scene;
std::vector<std::shared_ptr<odb::NativeBitmap>> textures;
//std::shared_ptr<odb::PETTable> petTable = std::make_shared<odb::PETTable>();
//std::shared_ptr<odb::GEOMap> geoMap = std::make_shared<odb::GEOMap>();

std::shared_ptr<Knights::CGame> game;
std::shared_ptr<odb::NoudarGLES2Bridge> render;

void loadShaders( std::string vertexShader, std::string fragmentShader ) {
	gVertexShader = vertexShader;
	gFragmentShader = fragmentShader;
}

bool setupGraphics(int w, int h, std::vector<std::shared_ptr<odb::NativeBitmap>> textureList ) {
	textures = textureList;
	std::stringstream meshData;
	std::stringstream materialData;

//	meshData << wavefrontCubeMesh;
//	materialData << wavefrontCubeMaterials;
//	scene = readScene( meshData, materialData );

	gles2Renderer = std::make_shared<odb::DungeonGLES2Renderer>();

//	auto it = scene->materialList.materials.begin();

//	while ( it != scene->materialList.materials.end() ) {
//		std::shared_ptr<odb::Material> material = it->second;
//		it = std::next( it );
//	}

	gles2Renderer->setTexture(textures);
	animationTime = 0;



	return gles2Renderer->init(w, h, gVertexShader.c_str(), gFragmentShader.c_str());
}

void renderFrame(long delta) {
	if (gles2Renderer != nullptr && textures.size() > 0) {

		gles2Renderer->updateFadeState(delta);
//		gles2Renderer->produceRenderingBatches( *geoMap, *petTable, map, snapshot, splat, lightMap, ids, animationList, animationTime );
		gles2Renderer->render(map, snapshot, splat, lightMap, ids, animationList, animationTime);
		gles2Renderer->updateCamera(delta);

//	    if ( scene != nullptr ) {
//		    auto it = scene->meshObjects.begin();
//		    while ( it != scene->meshObjects.end() ) {
//			    std::shared_ptr<odb::MeshObject> mesh = it->second;
//			    gles2Renderer->drawTrigBatch( mesh->trigBatches[0] );
//			    it = std::next( it );
//		    }
//	    }
	}
}

void shutdown() {
	gles2Renderer->shutdown();
	animationList.clear();
	mPositions.clear();
	animationTime = 0;
	textures.clear();
	hasCache = false;

	for (int y = 0; y < 20; ++y) {
		for (int x = 0; x < 20; ++x) {
			lightMapCache[y][x] = 0;
		}
	}

	gles2Renderer = nullptr;
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
}

void updateCharacterMovements(const int *idsLocal) {
	int position;

	for (int y = 0; y < 20; ++y) {
		for (int x = 0; x < 20; ++x) {
			position = (y * 20) + x;
			int id = idsLocal[position];
			ids[y][x] = id;
			if (id != 0) {
				auto previousPosition = mPositions[id];

				if (previousPosition != glm::vec2(x, y)) {
					mPositions[id] = glm::vec2(x, y);
					addCharacterMovement(id, previousPosition, mPositions[id]);
				}
			}
		}
	}
}

void setCameraPosition( int x, int y ) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setCameraPosition(x, y);
	}
}

void updateLevelSnapshot(const int *level, const int *actors, const int *splats) {
	hasActiveSplats = false;
	int position;

	for (int y = 0; y < 20; ++y) {
		for (int x = 0; x < 20; ++x) {
			position = (y * 20) + x;
			map[y][x] = (odb::ETextures) level[position];
			snapshot[y][x] = (odb::ETextures) actors[position];
			splat[y][x] = (odb::ETextures) splats[position];

			hasActiveSplats = hasActiveSplats || (splat[y][x] != -1);
			lightMap[y][x] = lightMapCache[y][x];
		}
	}

	for (int y = 0; y < 20; ++y) {
		for (int x = 0; x < 20; ++x) {

			if (map[y][x] == odb::BricksCandles) {

				if (!hasCache) {
					odb::LightningStrategy::castLightInAllDirections(lightMapCache, 255, map, x, y);
					odb::LightningStrategy::castLightInAllDirections(lightMap, 255, map, x, y);
				}
			}
		}
	}

	hasCache = true;
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

	std::lock_guard<std::mutex> lock(mutex);

	if (gles2Renderer != nullptr&& !isAnimating() ) {
		gles2Renderer->rotateLeft();
		render->setNextCommand('i');
		game->tick();
		render->setNextCommand('.');
	}
}

void rotateCameraRight() {

	std::lock_guard<std::mutex> lock(mutex);

	if (gles2Renderer != nullptr&& !isAnimating() ) {
		gles2Renderer->rotateRight();
		render->setNextCommand('p');
		game->tick();
		render->setNextCommand('.');
	}
}

void setFloorNumber( long floor ) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setFloorNumber(floor);
	}
}

void onReleasedLongPressingMove() {
	if (gles2Renderer != nullptr) {
		gles2Renderer->onReleasedLongPressingMove();
	}
}

void onLongPressingMove() {
	if (gles2Renderer != nullptr) {
		gles2Renderer->onLongPressingMove();
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

	std::stringstream geoStream;
	std::stringstream petStream;

	geoStream << geoFile;
	petStream << petFile;

//	geoMap = std::make_shared<odb::GEOMap>( geoStream );
//	petTable = std::make_shared<odb::PETTable>(petStream);
}

void readMap( std::string data ) {
	render = std::make_shared<odb::NoudarGLES2Bridge>();

	std::string::size_type n = 0;
	while ( ( n = data.find( "\n", n ) ) != std::string::npos )
	{
		data.replace( n, 1, "" );
		++n;
	}

	game = std::make_shared<Knights::CGame>( data, render );
	setFloorNumber(0);
	if ( game != nullptr ) {
		game->tick();
	}
}

void moveUp() {

	std::lock_guard<std::mutex> lock(mutex);

	if ( game != nullptr && !isAnimating() ) {
		render->setNextCommand('o');
		game->tick();
		render->setNextCommand('.');
	}
}

void moveDown() {

	std::lock_guard<std::mutex> lock(mutex);

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

	std::lock_guard<std::mutex> lock(mutex);

	if ( game != nullptr && !isAnimating() ) {
		render->setNextCommand('i');
		game->tick();
		render->setNextCommand('o');
		game->tick();
		render->setNextCommand('p');
		game->tick();
		render->setNextCommand('.');
	}
}

void moveRight() {

	std::lock_guard<std::mutex> lock(mutex);

	if ( game != nullptr && !isAnimating() ) {
		render->setNextCommand('p');
		game->tick();
		render->setNextCommand('o');
		game->tick();
		render->setNextCommand('i');
		game->tick();
		render->setNextCommand('.');
	}
}