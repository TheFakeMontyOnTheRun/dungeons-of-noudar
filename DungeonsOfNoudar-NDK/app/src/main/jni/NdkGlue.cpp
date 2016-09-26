/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
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

#include "gles2-renderer/NativeBitmap.h"
#include "gles2-renderer/Texture.h"
#include "gles2-renderer/Material.h"
#include "gles2-renderer/Trig.h"
#include "gles2-renderer/TrigBatch.h"
#include "gles2-renderer/MeshObject.h"
#include "gles2-renderer/MaterialList.h"
#include "gles2-renderer/Scene.h"
#include "gles2-renderer/WavefrontOBJReader.h"
#include "DungeonGLES2Renderer.h"

#include "NdkGlue.h"

#include "LightningStrategy.h"
#include "android_asset_operations.h"

std::string gVertexShader;
std::string gFragmentShader;

std::string wavefrontCubeMesh;
std::string wavefrontCubeMaterials;

std::shared_ptr<odb::DungeonGLES2Renderer> gles2Renderer = nullptr;
std::vector<std::shared_ptr<odb::NativeBitmap>> textures;
std::map< int, glm::vec2> mPositions;

long currentDelta = 0;

odb::IntGameMap map;
odb::IntGameMap snapshot;
odb::IntGameMap splat;
odb::IntField ids;
odb::LightMap lightMap;
odb::AnimationList animationList;
long animationTime = 0;
bool hasCache = false;
odb::LightMap lightMapCache;
std::shared_ptr<odb::Scene> scene;

void loadShaders(JNIEnv *env, jobject &obj) {
    AAssetManager *asset_manager = AAssetManager_fromJava(env, obj);
    FILE *fd;

    fd = android_fopen("vertex.glsl", "r", asset_manager);
    gVertexShader = readToString(fd);
    fclose(fd);

    fd = android_fopen("fragment.glsl", "r", asset_manager);
    gFragmentShader = readToString(fd);
    fclose(fd);

	fd = android_fopen("cubonormal.obj", "r", asset_manager);
	wavefrontCubeMesh = readToString(fd);
	fclose(fd);

	fd = android_fopen("cubonormal.mtl", "r", asset_manager);
	wavefrontCubeMaterials = readToString(fd);
	fclose(fd);
}

bool setupGraphics(int w, int h) {

	scene = readScene( wavefrontCubeMesh, wavefrontCubeMaterials );

    gles2Renderer = std::make_shared<odb::DungeonGLES2Renderer>();

	auto it = scene->materialList.materials.begin();

	while ( it != scene->materialList.materials.end() ) {
		std::shared_ptr<odb::Material> material = it->second;
//		textures.push_back( material->mBitmap );
		it = std::next( it );
	}

	gles2Renderer->setTexture(textures);
	animationTime = 0;
    return gles2Renderer->init(w, h, gVertexShader.c_str(), gFragmentShader.c_str());
}

void renderFrame(long delta) {
    if (gles2Renderer != nullptr && textures.size() > 0 ) {
	    gles2Renderer->updateFadeState(delta);
	    gles2Renderer->render(map, snapshot, splat, lightMap, ids, animationList, animationTime );
	    gles2Renderer->updateCamera( delta );


	    if ( scene != nullptr ) {
		    auto it = scene->meshObjects.begin();
		    while ( it != scene->meshObjects.end() ) {
			    std::shared_ptr<odb::MeshObject> mesh = it->second;
			    gles2Renderer->drawTrigBatch( mesh->trigBatches[0] );
			    it = std::next( it );
		    }
	    }
    }
}

void shutdown() {
	gles2Renderer->shutdown();
	animationList.clear();
	mPositions.clear();
	animationTime = 0;
	textures.clear();
	hasCache = false;

	for ( int y = 0; y < 20; ++y ) {
		for (int x = 0; x < 20; ++x) {
			lightMapCache[y][x] = 0;
		}
	}

	gles2Renderer = nullptr;
}

extern "C" {
JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onCreate(JNIEnv *env, void *reserved,
                                                                    jobject assetManager);
JNIEXPORT void JNICALL
        Java_br_odb_GL2JNILib_setYZAngle(JNIEnv *env, jclass type, jfloat yz);

JNIEXPORT void JNICALL
        Java_br_odb_GL2JNILib_setXZAngle(JNIEnv *env, jclass type, jfloat xz);

JNIEXPORT void JNICALL
        Java_br_odb_GL2JNILib_setPerspectiveMatrix(JNIEnv *env, jclass type,
                                                   jfloatArray perspectiveMatrix_);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setEyeMatrix(JNIEnv *env, jclass type, jfloatArray eyeView_);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setCurrentCursorPosition(JNIEnv *env, jclass type, jfloat x, jfloat y);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setCameraPosition(JNIEnv *env, jclass type, jfloat x, jfloat y);

JNIEXPORT void JNICALL
    Java_br_odb_GL2JNILib_setTextures(JNIEnv *env, jclass type, jobjectArray bitmaps);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setClearColour(JNIEnv *env, jclass type, jfloat r, jfloat g, jfloat b);


JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_rotateLeft(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_rotateRight(JNIEnv *env, jclass type);

JNIEXPORT jboolean JNICALL
Java_br_odb_GL2JNILib_isAnimating(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_toggleCloseupCamera(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onDestroy(JNIEnv *env, jobject obj);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setActorIdPositions(JNIEnv *env, jclass type, jintArray ids_);

JNIEXPORT void JNICALL
        Java_br_odb_GL2JNILib_tick(JNIEnv *env, jclass type, jlong delta);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_onReleasedLongPressingMove(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_onLongPressingMove(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_fadeOut(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_fadeIn(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_init(JNIEnv *env, jobject obj,
                                                                jint width, jint height);
JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_step(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_setFloorNumber(JNIEnv *env, jclass type, jlong floor);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setMapWithSplatsAndActors(JNIEnv *env, jclass type, jintArray map_, jintArray actors_, jintArray splats_);

};

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onCreate(JNIEnv *env, void *reserved,
                                                                    jobject assetManager) {
    loadShaders(env, assetManager);
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_init(JNIEnv *env, jobject obj,
                                                                jint width, jint height) {
    setupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_step(JNIEnv *env, jclass type) {
	renderFrame( currentDelta );
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onDestroy(JNIEnv *env, jobject obj) {
    shutdown();
}

std::shared_ptr<odb::NativeBitmap> makeNativeBitmapFromJObject(JNIEnv *env, jobject bitmap) {

	void *addr;
	AndroidBitmapInfo info;
	int errorCode;

	if ((errorCode = AndroidBitmap_lockPixels(env, bitmap, &addr)) != 0) {
		LOGI("error %d", errorCode);
	}

	if ((errorCode = AndroidBitmap_getInfo(env, bitmap, &info)) != 0) {
		LOGI("error %d", errorCode);
	}

	LOGI("bitmap info: %d wide, %d tall, %d ints per pixel", info.width, info.height, info.format);


	long size = info.width * info.height * info.format;
	int *pixels = new int[size];
	memcpy(pixels, addr, size * sizeof(int));
	auto toReturn = std::make_shared<odb::NativeBitmap>(info.width, info.height, pixels);

	if ((errorCode = AndroidBitmap_unlockPixels(env, bitmap)) != 0) {
		LOGI("error %d", errorCode);
	}

	return toReturn;
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setTextures(JNIEnv *env, jclass type, jobjectArray bitmaps) {
	int length = env->GetArrayLength( bitmaps );
	textures.clear();
	for ( int c = 0; c < length; ++c ) {
		textures.push_back( makeNativeBitmapFromJObject( env, env->GetObjectArrayElement( bitmaps, c ) ) );
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setCameraPosition(JNIEnv *env, jclass type, jfloat x, jfloat y) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setCameraPosition( x, y );
	}
}


JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setMapWithSplatsAndActors(JNIEnv *env, jclass type, jintArray map_, jintArray actors_, jintArray splats_) {
	jint *level = env->GetIntArrayElements(map_, NULL);
	jint *actors = env->GetIntArrayElements(actors_, NULL);
	jint *splats = env->GetIntArrayElements(splats_, NULL);

	int position;
	for ( int y = 0; y < 20; ++y ) {
		for ( int x = 0; x < 20; ++x ) {
			position = ( y * 20 ) + x;
			map[ y ][ x ] = (odb::ETextures) level[ position ];
			snapshot[ y ][ x ] = (odb::ETextures) actors[ position ];
			splat[ y ][ x ] = (odb::ETextures) splats[ position ];
			lightMap[ y ][ x ] = lightMapCache[ y ][ x ];
		}
	}

	for ( int y = 0; y < 20; ++y ) {
		for (int x = 0; x < 20; ++x) {

			if ( map[ y ][ x ] == odb::ETextures::BricksCandles ) {

				if ( !hasCache ) {
					odb::LightningStrategy::castLightInAllDirections( lightMapCache, 255, map, x, y);
					odb::LightningStrategy::castLightInAllDirections( lightMap, 255, map, x, y);
				}
			}

			//splat?
			if ( snapshot[ y ][ x ] != odb::ETextures::None ) {
//				odb::LightningStrategy::castPointLight( lightMap, 16, map, x, y);
			}

		}
	}

	hasCache = true;

	env->ReleaseIntArrayElements(map_, level, 0);
	env->ReleaseIntArrayElements(actors_, actors, 0);
	env->ReleaseIntArrayElements(splats_, splats, 0);
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setCurrentCursorPosition(JNIEnv *env, jclass type, jfloat x, jfloat y) {

	if (gles2Renderer != nullptr) {
		gles2Renderer->setCursorAt( x, y );
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_toggleCloseupCamera(JNIEnv *env, jclass type) {

	if (gles2Renderer != nullptr) {
		gles2Renderer->toggleCloseUpCamera();
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setClearColour(JNIEnv *env, jclass type, jfloat r, jfloat g, jfloat b) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setClearColour( r, g, b );
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_fadeIn(JNIEnv *env, jclass type) {

	if (gles2Renderer != nullptr) {
		gles2Renderer->startFadingIn();
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_fadeOut(JNIEnv *env, jclass type) {

	if (gles2Renderer != nullptr) {
		gles2Renderer->startFadingOut();
	}
}

JNIEXPORT jboolean JNICALL
Java_br_odb_GL2JNILib_isAnimating(JNIEnv *env, jclass type) {

	if (gles2Renderer != nullptr) {
		return gles2Renderer->isAnimating();
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_rotateLeft(JNIEnv *env, jclass type) {

	if (gles2Renderer != nullptr) {
		gles2Renderer->rotateLeft();
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_rotateRight(JNIEnv *env, jclass type) {

	if (gles2Renderer != nullptr) {
		gles2Renderer->rotateRight();
	}
}



void addCharacterMovement( int id, glm::vec2 previousPosition, glm::vec2 newPosition ) {
	auto movement =  std::make_tuple<>(previousPosition, newPosition, animationTime );

	if ( animationList.count( id ) > 0 ) {

		auto animation = animationList[id];
		auto prevPosition = std::get<0>(animation);
		auto destPosition = std::get<1>(animation);
		animation = std::make_tuple<>(prevPosition, newPosition, animationTime );
	}

	animationList[ id ] = movement;
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setActorIdPositions(JNIEnv *env, jclass type, jintArray ids_) {
	jint *idsLocal = env->GetIntArrayElements(ids_, NULL);

	if (gles2Renderer == nullptr) {
		return;
	}

	int position;
    for ( int y = 0; y < 20; ++y ) {
    	for ( int x = 0; x < 20; ++x ) {
    		position = ( y * 20 ) + x;
    		int id = idsLocal[ position ];
			ids[ y ][ x ] = id;
			if ( id != 0 ) {
				auto previousPosition = mPositions[ id ];

				if ( previousPosition != glm::vec2( x, y ) ) {
					mPositions[ id ] = glm::vec2( x, y );
					addCharacterMovement( id, previousPosition, mPositions[ id ] );
				}
			}
    	}
    }

	env->ReleaseIntArrayElements(ids_, idsLocal, 0);
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_setFloorNumber(JNIEnv *env, jclass type, jlong floor) {
	if (gles2Renderer != nullptr) {
		gles2Renderer->setFloorNumber( floor );
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_onReleasedLongPressingMove(JNIEnv *env, jclass type) {

	if (gles2Renderer != nullptr) {
		gles2Renderer->onReleasedLongPressingMove();
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_onLongPressingMove(JNIEnv *env, jclass type) {

	if (gles2Renderer != nullptr) {
		gles2Renderer->onLongPressingMove();
	}
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setEyeMatrix(JNIEnv *env, jclass type, jfloatArray eyeView_) {
	jfloat *eyeView = env->GetFloatArrayElements(eyeView_, NULL);

    if (gles2Renderer != nullptr) {
        gles2Renderer->setEyeView( eyeView );
    }

	env->ReleaseFloatArrayElements(eyeView_, eyeView, 0);
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_tick(JNIEnv *env, jclass type, jlong delta) {

    auto it = animationList.begin();
    while ( it != animationList.end() ) {
        if ( animationTime - (std::get<2>(it->second)) >= odb::kAnimationLength ) {
            it = animationList.erase( it );
        } else {
            it = std::next( it );
        }
    }

    animationTime += delta;
    currentDelta = delta;
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setPerspectiveMatrix(JNIEnv *env, jclass type,
                                           jfloatArray perspectiveMatrix_) {
    jfloat *perspectiveMatrix = env->GetFloatArrayElements(perspectiveMatrix_, NULL);

    if (gles2Renderer != nullptr) {
        gles2Renderer->setPerspectiveMatrix( perspectiveMatrix );
    }

    env->ReleaseFloatArrayElements(perspectiveMatrix_, perspectiveMatrix, 0);
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setXZAngle(JNIEnv *env, jclass type, jfloat xz) {

    if (gles2Renderer != nullptr) {
        gles2Renderer->setAngleXZ( xz );
    }
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setYZAngle(JNIEnv *env, jclass type, jfloat yz) {

    if (gles2Renderer != nullptr) {
        gles2Renderer->setAngleYZ( yz );
    }
}