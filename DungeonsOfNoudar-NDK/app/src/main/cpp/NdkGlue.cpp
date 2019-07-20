#include <glm/glm.hpp>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>
#include "NdkGlue.h"

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <array>

using std::vector;
using std::array;

#include "Common.h"
#include "Logger.h"
#include "NativeBitmap.h"
#include "android_asset_operations.h"
#include "AudioSink.h"
#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "IFileLoaderDelegate.h"
#include "AndroidFileLoaderDelegate.h"


#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"


#include "NoudarDungeonSnapshot.h"

#include "GameNativeAPI.h"
#include "AndroidAudioSink.h"

using Knights::readToString;

vector<std::shared_ptr<odb::NativeBitmap>> texturesToLoad;
vector<std::shared_ptr<odb::SoundEmitter>> sounds;
vector< std::string> meshes;
std::string gVertexShader;
std::string gFragmentShader;

std::shared_ptr<AndroidAudioSink> audioSinkWrapper;
std::shared_ptr<odb::SoundListener> soundListener;

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags,
                     const char* file, int line) {
    return malloc( size );
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName,
                     int flags, unsigned debugFlags, const char* file, int line) {
    return malloc( size );
}

std::shared_ptr<odb::SoundEmitter> makeSoundEmitterFromFilename(JNIEnv *env, jclass type,
                                                                AAssetManager *assetManager,
                                                                jstring filename) {
	FILE *soundFile;

	const char *rawString = env->GetStringUTFChars(filename, 0);

	soundFile = android_fopen(rawString, "r", assetManager);
	auto soundClip = odb::makeSoundClipFrom(audioSinkWrapper, soundFile);
	fclose(soundFile);

	return std::make_shared<odb::SoundEmitter>(audioSinkWrapper, soundClip);
}

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

	odb::Logger::log("bitmap info: %d wide, %d tall, %d ints per pixel", info.width, info.height,
	                 info.format);


	long size = info.width * info.height * info.format;
	int *pixels = new int[size];
	memcpy(pixels, addr, size * sizeof(int));

	auto toReturn = std::make_shared<odb::NativeBitmap>("", info.width, info.height, pixels);

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

void loadSoundsFromFilenames(JNIEnv *env, jclass type, jobject asset_manager,
                             _jobjectArray *soundFiles, jsize length) {
	sounds.clear();
	AAssetManager *assetManager = AAssetManager_fromJava(env, asset_manager);

	for (int c = 0; c < length; ++c) {

		sounds.push_back(makeSoundEmitterFromFilename(env, type, assetManager,
		                                              (jstring) (env->GetObjectArrayElement(
				                                              soundFiles, c))));
	}
}

extern "C" {

JNIEXPORT jboolean JNICALL
Java_br_odb_GL2JNILib_isThereAnyObjectInFrontOfYou(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_cyclePreviousItem(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_cycleNextItem(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_dropItem(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_pickItem(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_useItem(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_loadSounds(JNIEnv *env, jclass type,
		                                 jobject asset_manager, jobjectArray soundFiles);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_flush(JNIEnv *env, jclass type, jobject sink);

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onCreate(JNIEnv *env, jclass type,
                                                      jobject assetManager);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setTextures(JNIEnv *env, jclass type, jobjectArray bitmaps);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setClearColour(JNIEnv *env, jclass type, jfloat r, jfloat g,
		                                     jfloat b);
JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_moveUp(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_moveDown(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_moveLeft(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_moveRight(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_rotateLeft(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_rotateRight(JNIEnv *env, jclass type);

JNIEXPORT jboolean JNICALL
		Java_br_odb_GL2JNILib_isAnimating(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onDestroy(JNIEnv *env, jclass type);

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

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_init(JNIEnv *env, jclass type,
                                                  jint width, jint height, jobject asset_manager);

JNIEXPORT void JNICALL
		Java_br_odb_GL2JNILib_setMapWithSplatsAndActors(JNIEnv *env, jclass type, jintArray map_,
		                                                jintArray actors_, jintArray splats_);

JNIEXPORT jint JNICALL
    Java_br_odb_GL2JNILib_getLevel(JNIEnv *env, jclass type);

JNIEXPORT jboolean JNICALL
    Java_br_odb_GL2JNILib_isPlaying(JNIEnv *env, jclass type);

};

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onCreate(JNIEnv *env, jclass type,
                                                      jobject assetManager) {

	AAssetManager *asset_manager = AAssetManager_fromJava(env, assetManager);
	FILE *fd;

	fd = android_fopen("vertex.glsl", "r", asset_manager);
	gVertexShader = readToString(fd);
	fclose(fd);

	fd = android_fopen("fragment.glsl", "r", asset_manager);
	gFragmentShader = readToString(fd);
	fclose(fd);

	readMap(std::make_shared<odb::AndroidFileLoaderDelegate>(asset_manager));
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_init(JNIEnv *env, jclass type,
                                                  jint width, jint height, jobject assets) {

    AAssetManager *assetManager = AAssetManager_fromJava(env, assets);
    auto loader = std::make_shared<odb::AndroidFileLoaderDelegate>(assetManager);

    setupGraphics(width, height, gVertexShader, gFragmentShader, loader);
}

JNIEXPORT void JNICALL Java_br_odb_GL2JNILib_onDestroy(JNIEnv *env, jclass type) {
	shutdown();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_setTextures(JNIEnv *env, jclass type, jobjectArray bitmaps) {
	loadTexturesFromBitmaps(env, bitmaps, env->GetArrayLength(bitmaps));
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
Java_br_odb_GL2JNILib_onReleasedLongPressingMove(JNIEnv *env, jclass type) {
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_onLongPressingMove(JNIEnv *env, jclass type) {

}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_tick(JNIEnv *env, jclass type, jlong delta) {
    gameLoopTick(20);
    renderFrame(20);
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_moveUp(JNIEnv *env, jclass type) {

	moveUp();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_moveDown(JNIEnv *env, jclass type) {

	moveDown();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_moveLeft(JNIEnv *env, jclass type) {

	moveLeft();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_moveRight(JNIEnv *env, jclass type) {

	moveRight();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_flush(JNIEnv *env, jclass type, jobject sink) {
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_loadSounds(JNIEnv *env, jclass type, jobject asset_manager,
                                 jobjectArray soundFiles) {

	audioSinkWrapper = std::make_shared<AndroidAudioSink>();
	soundListener = std::make_shared<odb::SoundListener>(audioSinkWrapper);

	loadSoundsFromFilenames(env, type, asset_manager, soundFiles,
	                        env->GetArrayLength(soundFiles));

	setSoundEmitters( sounds, soundListener );
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_cyclePreviousItem(JNIEnv *env, jclass type) {
    cyclePrevItem();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_cycleNextItem(JNIEnv *env, jclass type) {
    cycleNextItem();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_dropItem(JNIEnv *env, jclass type) {
    dropItem();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_pickItem(JNIEnv *env, jclass type) {
    pickupItem();
}

JNIEXPORT void JNICALL
Java_br_odb_GL2JNILib_useItem(JNIEnv *env, jclass type) {
    interact();
}

JNIEXPORT jboolean JNICALL
Java_br_odb_GL2JNILib_isThereAnyObjectInFrontOfYou(JNIEnv *env, jclass type) {

    return isThereAnyObjectInFrontOfYou();
}

JNIEXPORT jint JNICALL
Java_br_odb_GL2JNILib_getLevel(JNIEnv *env, jclass type) {

    return getLevel();
}