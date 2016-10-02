#include <glm/glm.hpp>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>

#include <string>
#include <memory>
#include <vector>

#include "gles2-renderer/Logger.h"
#include "gles2-renderer/NativeBitmap.h"
#include "android_asset_operations.h"
#include "GameNativeAPI.h"

long currentDelta = 0;
std::vector<std::shared_ptr<odb::NativeBitmap>> texturesToLoad;

std::shared_ptr<odb::NativeBitmap> makeNativeBitmapFromJObject(JNIEnv *env, jobject bitmap) {

	void *addr;
	AndroidBitmapInfo info;
	int errorCode;

	if ((errorCode = AndroidBitmap_lockPixels(env, bitmap, &addr)) != 0) {
		odb::Logger::log("error %d", errorCode);
	}

	if ((errorCode = AndroidBitmap_getInfo(env, bitmap, &info)) != 0) {
		odb::Logger::log("error %d", errorCode);
	}

	odb::Logger::log("bitmap info: %d wide, %d tall, %d ints per pixel", info.width, info.height, info.format);


	long size = info.width * info.height * info.format;
	int *pixels = new int[size];
	memcpy(pixels, addr, size * sizeof(int));

	auto toReturn = std::make_shared<odb::NativeBitmap>(info.width, info.height, pixels);

	if ((errorCode = AndroidBitmap_unlockPixels(env, bitmap)) != 0) {
		odb::Logger::log("error %d", errorCode);
	}

	return toReturn;
}

void loadTexturesFromBitmaps(JNIEnv *env, _jobjectArray *bitmaps, int length) {
	texturesToLoad.clear();
	for (int c = 0; c < length; ++c) {
		texturesToLoad.push_back(
				makeNativeBitmapFromJObject(env, env->GetObjectArrayElement(bitmaps, c)));
	}
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
		Java_br_odb_GL2JNILib_setCameraPosition(JNIEnv *env, jclass type, jfloat x, jfloat y);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setTextures(JNIEnv *env, jclass type, jobjectArray bitmaps);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setClearColour(JNIEnv *env, jclass type, jfloat r, jfloat g,
		                                     jfloat b);


JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_rotateLeft(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_rotateRight(JNIEnv *env, jclass type);

JNIEXPORT jboolean JNICALL
		Java_br_odb_GL2JNILib_isAnimating(JNIEnv *env, jclass type);

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
		Java_br_odb_GL2JNILib_setMapWithSplatsAndActors(JNIEnv *env, jclass type, jintArray map_,
		                                                jintArray actors_, jintArray splats_);

};

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onCreate(JNIEnv *env, void *reserved,
                                                      jobject assetManager) {

	AAssetManager *asset_manager = AAssetManager_fromJava(env, assetManager);
	FILE *fd;

	fd = android_fopen("vertex.glsl", "r", asset_manager);
	std::string gVertexShader = readToString(fd);
	fclose(fd);

	fd = android_fopen("fragment.glsl", "r", asset_manager);
	std::string gFragmentShader = readToString(fd);


	loadShaders(gVertexShader, gFragmentShader);
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_init(JNIEnv *env, jobject obj,
                                                  jint width, jint height) {
	setupGraphics(width, height, texturesToLoad );
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_step(JNIEnv *env, jclass type) {
	renderFrame(currentDelta);
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onDestroy(JNIEnv *env, jobject obj) {
	shutdown();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setTextures(JNIEnv *env, jclass type, jobjectArray bitmaps) {
	loadTexturesFromBitmaps(env, bitmaps, env->GetArrayLength(bitmaps));
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setCameraPosition(JNIEnv *env, jclass type, jfloat x, jfloat y) {
	setCameraPosition( x, y );
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setMapWithSplatsAndActors(JNIEnv *env, jclass type, jintArray map_,
                                                jintArray actors_, jintArray splats_) {
	jint *level = env->GetIntArrayElements(map_, NULL);
	jint *actors = env->GetIntArrayElements(actors_, NULL);
	jint *splats = env->GetIntArrayElements(splats_, NULL);

	updateLevelSnapshot(level, actors, splats);

	env->ReleaseIntArrayElements(map_, level, 0);
	env->ReleaseIntArrayElements(actors_, actors, 0);
	env->ReleaseIntArrayElements(splats_, splats, 0);
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_fadeIn(JNIEnv *env, jclass type) {
	startFadingIn();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_fadeOut(JNIEnv *env, jclass type) {
	startFadingOut();
}

JNIEXPORT jboolean JNICALL
Java_br_odb_GL2JNILib_isAnimating(JNIEnv *env, jclass type) {
	return isAnimating();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_rotateLeft(JNIEnv *env, jclass type) {
	rotateCameraLeft();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_rotateRight(JNIEnv *env, jclass type) {
	rotateCameraRight();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setActorIdPositions(JNIEnv *env, jclass type, jintArray ids_) {

	jint *idsLocal = env->GetIntArrayElements(ids_, NULL);

	updateCharacterMovements(idsLocal);

	env->ReleaseIntArrayElements(ids_, idsLocal, 0);
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_setFloorNumber(JNIEnv *env, jclass type, jlong floor) {
	setFloorNumber( floor );
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_onReleasedLongPressingMove(JNIEnv *env, jclass type) {
	onReleasedLongPressingMove();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_onLongPressingMove(JNIEnv *env, jclass type) {
	onLongPressingMove();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setEyeMatrix(JNIEnv *env, jclass type, jfloatArray eyeView_) {
	jfloat *eyeView = env->GetFloatArrayElements(eyeView_, NULL);

	setEyeViewMatrix( eyeView );

	env->ReleaseFloatArrayElements(eyeView_, eyeView, 0);
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_tick(JNIEnv *env, jclass type, jlong delta) {

	updateAnimations( delta );
	currentDelta = delta;
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setPerspectiveMatrix(JNIEnv *env, jclass type,
                                           jfloatArray perspectiveMatrix_) {
	jfloat *perspectiveMatrix = env->GetFloatArrayElements(perspectiveMatrix_, NULL);

	setPerspectiveMatrix( perspectiveMatrix );

	env->ReleaseFloatArrayElements(perspectiveMatrix_, perspectiveMatrix, 0);
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setXZAngle(JNIEnv *env, jclass type, jfloat xz) {

	setAngleXZ( xz );
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setYZAngle(JNIEnv *env, jclass type, jfloat yz) {

	setAngleYZ( yz );
}