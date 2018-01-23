#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <utility>
#include <functional>
#include <memory>
#include <algorithm>
#include <iostream>
#include <map>
#include <unordered_map>
#include <chrono>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <sg14/fixed_point>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;
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
#include "CMap.h"
#include "IRenderer.h"
#include "IFileLoaderDelegate.h"
#include "CGame.h"
#include "NativeBitmap.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "LoadPNG.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

bool kbhit() {

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            SDL_PushEvent(&event);
            return true;
        }
    }

    return false;
}

long timeEllapsed = 0;

long uclock() {
    timeEllapsed += 33;
    return timeEllapsed;
}

int getch() {

    SDL_Event event;

    while (true) {
        if (SDL_PollEvent(&event) ) {
            if (event.type == SDL_KEYDOWN) {
                return event.key.keysym.scancode;
            }
        }
    }
}


namespace odb {
//    bool drawZBuffer = false;
    SDL_Surface *video;

#ifdef __EMSCRIPTEN__
    void enterFullScreenMode() {
    EmscriptenFullscreenStrategy s;
    memset(&s, 0, sizeof(s));
    s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
    s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
    s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    emscripten_enter_soft_fullscreen(0, &s);
}
#endif


    CRenderer::CRenderer() {
        SDL_Init(SDL_INIT_EVERYTHING);
        video = SDL_SetVideoMode(640, 400, 32, 0);

        for (int r = 0; r < 256; r += 16) {
            for (int g = 0; g < 256; g += 8) {
                for (int b = 0; b < 256; b += 8) {
                    auto pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                    auto paletteEntry = getPaletteEntry(pixel);
                    mPalette[paletteEntry] = pixel;
                }
            }
        }

#ifdef __EMSCRIPTEN__
        enterFullScreenMode();
#endif
    }

    void CRenderer::sleep(long ms) {
#ifndef __EMSCRIPTEN__
//        SDL_Delay(100);
//        timeEllapsed += 100;
#endif
    }

    void CRenderer::handleSystemEvents() {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
#ifndef __EMSCRIPTEN__
                mBufferedCommand = Knights::kQuitGameCommand;
#endif
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        mBufferedCommand = Knights::kQuitGameCommand;
                        break;
                    case SDLK_SPACE:
                        mBufferedCommand = Knights::kUseCurrentItemInInventoryCommand;
                        mCached = false;
                        break;

                    case SDLK_w:
                        mBufferedCommand = Knights::kPickItemCommand;
                        mCached = false;
                        break;

                    case SDLK_s:
                        mBufferedCommand = Knights::kDropItemCommand;
                        mCached = false;
                        break;

                    case SDLK_e:
                        mBufferedCommand = Knights::kCycleRightInventoryCommand;
                        mCached = false;
                        break;

                    case SDLK_q:
                        mBufferedCommand = Knights::kCycleRightInventoryCommand;
                        mCached = false;
                        break;

                    case SDLK_LEFT:
                        mBufferedCommand = Knights::kTurnPlayerLeftCommand;
                        mCached = false;
                        break;
                    case SDLK_RIGHT:
                        mBufferedCommand = Knights::kTurnPlayerRightCommand;
                        mCached = false;
                        break;
                    case SDLK_UP:
                        mBufferedCommand = Knights::kMovePlayerForwardCommand;
                        mCached = false;
                        break;
                    case SDLK_DOWN:
                        mBufferedCommand = Knights::kMovePlayerBackwardCommand;
                        mCached = false;
                        break;

                    case SDLK_d:
                    case SDLK_a:
//                        drawZBuffer = (event.key.keysym.sym == SDLK_a);
                        mCached = false;
                        break;

                    case SDLK_z:
                        mBufferedCommand = Knights::kStrafeLeftCommand;
                        mCached = false;
                        break;

                    case SDLK_x:
                        mBufferedCommand = Knights::kStrafeRightCommand;
                        mCached = false;
                        break;

                    case SDLK_n:
                        mSlow = true;
                        mCached = false;
                        break;

                    case SDLK_m:
                        mSlow = false;
                        mCached = false;
                        break;


                    default:
                        return;
                }
            }
        }
    }

    void CRenderer::putRaw(int16_t x, int16_t y, uint32_t pixel) {


        if (x < 0 || x >= 256 || y < 0 || y >= 128) {
            return;
        }

        mBuffer[(320 * y) + x] = pixel;
    }

    void CRenderer::drawTextAt( int x, int y, const char* text ) {
        std::cout << "text: " << text << std::endl;
    }

    CRenderer::~CRenderer() {
        SDL_FreeSurface(video);
        SDL_Quit();
    }

    void CRenderer::flip() {
//        if (drawZBuffer) {
//
//            float smaller = 0.0f;
//            float bigger = 0.0f;
//
//            for ( const auto& value : mDepthBuffer ) {
//                float fv = static_cast<float>(value);
//                if ( fv < smaller ) {
//                    smaller = fv;
//                }
//
//                if ( fv > bigger && fv < 255.0f ) {
//                    bigger = fv;
//                }
//            }
//
//            float range = bigger - smaller;
//            float greys = 255.0f;
//
//            for (int y = 0; y < 200; ++y) {
//                for (int x = 0; x < 320; ++x) {
//                    SDL_Rect rect;
//                    rect.x = 2 * x;
//                    rect.y = 2 * y;
//                    rect.w = 2;
//                    rect.h = 2;
//                    float depth = static_cast<float>(mDepthBuffer[(320 * y) + x]);
//
//                    auto value = ((depth - smaller) / range) * greys;
//
//                    auto pixel = static_cast<uint8_t >(value);
//
//                    SDL_FillRect(video, &rect,
//                                 SDL_MapRGB(video->format, ((pixel & 0x000000FF)), ((pixel & 0x0000FF00) >> 8),
//                                            ((pixel & 0x00FF0000) >> 16)));
//
//                }
//            }
//        } else {
            for (int y = 0; y < 200; ++y) {
                for (int x = 0; x < 320; ++x) {
                    SDL_Rect rect;
                    rect.x = 2 * x;
                    rect.y = 2 * y;
                    rect.w = 2;
                    rect.h = 2;

                    auto pixel = mPalette[mBuffer[(320 * y) + x]];

                    SDL_FillRect(video, &rect,
                                 SDL_MapRGB(video->format, ((pixel & 0x000000FF)), ((pixel & 0x0000FF00) >> 8),
                                            ((pixel & 0x00FF0000) >> 16)));

                }
            }
//        }



        SDL_Flip(video);
    }

    void CRenderer::clear() {
        SDL_FillRect(video, nullptr, 0);
    }
}
