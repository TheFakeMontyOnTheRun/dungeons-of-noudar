#include <jni.h>
#include <string.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <memory>
#include <utility>
#include <map>
#include <algorithm>
#include <chrono>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <utility>
#include <functional>
#include <memory>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <chrono>

#include <sg14/fixed_point>
#include <vector>
#include <array>


enum class ESoundDriver {
    kNone, kPcSpeaker, kOpl2Lpt, kAdlib, kTandy, kCovox
};

ESoundDriver soundDriver = ESoundDriver::kNone;

void initOPL2(int port);

void playTune(const std::string &);

///game interface
void init();

void loopTick(long ms);

void shutdown();

bool isPlaying();
///

using std::vector;
using std::array;
using namespace std::chrono;
using sg14::fixed_point;

#include "RasterizerCommon.h"
#include "RaycastCommon.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "MapWithCharKey.h"
#include "CMap.h"
#include "IRenderer.h"
#include "IFileLoaderDelegate.h"
#include "CGame.h"
#include "NativeBitmap.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "LoadPNG.h"


char commandBuffer = '.';
uint8_t framebufferFinal[320 * 240 * 4];

AAssetManager *defaultAssetManager = NULL;
#define TRANSPARENCY_COLOR 119

namespace odb {

    long frame = 0;

/*
    uint8_t CRenderer::getPaletteEntry(const uint32_t origin) {
        uint8_t shade;

        if (!(origin & 0xFF000000)) {
            return TRANSPARENCY_COLOR;
        }

        shade = 0;
        shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
        shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
        shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

        return shade;
    }
*/
    void CRenderer::putRaw(int16_t x, int16_t y, uint32_t pixel) {

        if (x < 0 || x >= 320 || y < 0 || y >= 128) {
            return;
        }

        mBuffer[(320 * y) + x] = pixel;
    }

    CRenderer::~CRenderer() {
        mNativeTextures.clear();
    }


    CRenderer::CRenderer(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {

        for (int r = 0; r < 256; r += 16) {
            for (int g = 0; g < 256; g += 8) {
                for (int b = 0; b < 256; b += 8) {
                    auto pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                    auto paletteEntry = getPaletteEntry(pixel);
                    mPalette[paletteEntry] = pixel;
                }
            }
        }


        mFont = loadPNG("font.png", fileLoader);
    }

    void CRenderer::sleep(long ms) {
    }

    void CRenderer::handleSystemEvents() {
#ifdef PROFILEBUILD
        gotoxy(1,1);
        printf("%d", ++mFrame);
#endif
        const static FixP delta{2};

        int lastKey = 0;

        if (true) {
            auto getched = commandBuffer;
            commandBuffer = '.';
            switch (getched) {
                case '\n':
                    mBufferedCommand = Knights::kStartCommand;
                    break;

                case 'z':
                    mBufferedCommand = Knights::kPickItemCommand;
                    mCached = false;
                    break;
                case 'x':
                    mBufferedCommand = Knights::kCycleRightInventoryCommand;
                    mCached = false;
                    break;

                case 'a':
                    mBufferedCommand = Knights::kStrafeLeftCommand;
                    mCached = false;
                    break;
                case 'd':
                    mBufferedCommand = Knights::kStrafeRightCommand;
                    mCached = false;
                    break;

                case '1':
                case '2':
                case '3':
                    mBufferedCommand = getched;
                    mCached = false;
                    break;


                case 'c':
                    mBufferedCommand = Knights::kUseCurrentItemInInventoryCommand;
                    mCached = false;
                    break;

                case 'q':
                    mBufferedCommand = Knights::kTurnPlayerLeftCommand;
                    mCached = false;
                    break;
                case 'w':
                    mBufferedCommand = Knights::kMovePlayerForwardCommand;
                    mCached = false;
                    break;
                case 'e':
                    mBufferedCommand = Knights::kTurnPlayerRightCommand;
                    mCached = false;
                    break;
                case 's':
                    mBufferedCommand = Knights::kMovePlayerBackwardCommand;
                    mCached = false;
                    break;
            }
        }
    }

    void CRenderer::flip() {
        unsigned char *ptr = &framebufferFinal[0];

        memset(ptr, 0, 320 * 240 * 4);

        for (int y = 0; y < 240; ++y) {
            for (int x = 0; x < 320; ++x) {

                int32_t pixel = mPalette[mBuffer[(int) (320 * ((200 * y) / 240)) + x]];

                int r = (pixel & 0x000000FF) - 0x38;
                int g = ((pixel & 0x0000FF00) >> 8) - 0x18;
                int b = ((pixel & 0x00FF0000) >> 16) - 0x10;


                *ptr = r;
                ++ptr;
                *ptr = g;
                ++ptr;
                *ptr = b;
                ++ptr;
                *ptr = 255;
                ++ptr;

            }
        }
    }

    void CRenderer::clear() {
    }


    bool peekKeyboard(std::shared_ptr<CRenderer> unused) {
        return commandBuffer != '.';
    }

    int readKeyboard(std::shared_ptr<CRenderer> unused) {
        return commandBuffer;
    }

}

int soundToPlay = -1;

void setupOPL2(int port) {}

void stopSounds() {}

void playSound(const int action) {
    soundToPlay = action;
}

void playTune(const std::string &sound) {
    if ( sound == "i114t1o8f") {
        soundToPlay = 0;
    } else if ( sound == "t200i101o3afo1a") {
        soundToPlay = 1;
    } else if ( sound == "t200i101o8ao4ao2ao1a") {
        soundToPlay = 2;
    } else if ( sound == "t200i52o4defg") {
        soundToPlay = 3;
    } else if ( sound == "t200i53o3fo1f") {
        soundToPlay = 4;
    } else if ( sound == "t200i98a") {
        soundToPlay = 5;
    } else if ( sound == "t200i9o1fa") {
        soundToPlay = 6;
    }
}

void soundTick() {}

extern "C"
JNIEXPORT void JNICALL
Java_pt_b13h_noudar_NoudarJNI_initAssets(JNIEnv *env, jclass clazz,
                                                   jobject assetManager) {
    AAssetManager *asset_manager = AAssetManager_fromJava(env, assetManager);
    defaultAssetManager = asset_manager;
    srand(time(NULL));
    init();
}

extern "C"
JNIEXPORT void JNICALL
Java_pt_b13h_noudar_NoudarJNI_getPixelsFromNative(JNIEnv *env, jclass clazz,
                                                            jbyteArray java_side) {
    loopTick(75);
    jbyte *narr = (env)->GetByteArrayElements(java_side, NULL);
    memcpy(narr, &framebufferFinal[0], 320 * 240 * 4);
}

extern "C"
JNIEXPORT void JNICALL
Java_pt_b13h_noudar_NoudarJNI_sendCommand(JNIEnv *env, jclass clazz, jchar cmd) {
    commandBuffer = cmd;
}

extern "C"
JNIEXPORT jint JNICALL
Java_pt_b13h_noudar_NoudarJNI_getSoundToPlay(JNIEnv *env, jclass clazz) {
    int toReturn = soundToPlay;
    soundToPlay = -1;
    return toReturn;
}