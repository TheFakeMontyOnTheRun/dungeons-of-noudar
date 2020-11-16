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

#ifdef __APPLE__
#include "SDL.h"
#else
#include <SDL/SDL.h>
#endif

SDL_Window *window;
SDL_Renderer *SDLrenderer;
SDL_AudioDeviceID deviceId;

#include <sg14/fixed_point>
#include <vector>
#include <array>

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
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "LoadPNG.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif


long timeEllapsed = 0;

long uclock() {
    timeEllapsed += 4 * (1000/60);
    return timeEllapsed;
}

namespace odb {

  int readKeyboard(std::shared_ptr<CRenderer> renderer) {

        SDL_Event event;

        while (true) {
#ifdef __APPLE__
	  renderer->flip();
#endif
            if (SDL_PollEvent(&event) ) {
                if (event.type == SDL_KEYDOWN) {
                    if ( event.key.keysym.sym == SDLK_RETURN ) {
                        return Knights::kStartCommand;
                    } else {
                        return 0;
                    }
                }
            }
        }
    }

  bool peekKeyboard(std::shared_ptr<CRenderer> renderer) {

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
#ifdef __APPLE__
	  renderer->flip();
#endif
            if (event.type == SDL_KEYDOWN) {
                SDL_PushEvent(&event);
                return true;
            }
        }

        return false;
    }


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


    CRenderer::CRenderer(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {
        SDL_Init(SDL_INIT_EVERYTHING);

        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

        window =
                SDL_CreateWindow("The Mistral Report", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

        SDLrenderer = SDL_CreateRenderer(window, -1, 0);


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
        mFont = loadPNG("font.png", fileLoader );
    }

    void CRenderer::sleep(long ms) {
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
                    case SDLK_RETURN:
                        mBufferedCommand = Knights::kStartCommand;
                        break;

                    case SDLK_ESCAPE:
                        mBufferedCommand = Knights::kQuitGameCommand;
                        break;
                    case SDLK_SPACE:
                        mBufferedCommand = Knights::kUseCurrentItemInInventoryCommand;
                        mCached = false;
                        break;

                    case SDLK_a:
                        mBufferedCommand = Knights::kPickItemCommand;
                        mCached = false;
                        break;

                    case SDLK_s:
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

                    case SDLK_1:
                        mBufferedCommand = '1';
                        mCached = false;
                        break;
                    case SDLK_2:
                        mBufferedCommand = '2';
                        mCached = false;
                        break;
                    case SDLK_3:
                        mBufferedCommand = '3';
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



    CRenderer::~CRenderer() {
        SDL_Quit();

        mNativeTextures.clear();
    }

    void CRenderer::flip() {

        SDL_Rect rect;
        uint32_t pixel;
        int x, y;

        for (y = 0; y < 200; ++y) {
            for (x = 0; x < 320; ++x) {

                rect.x = 2 * x;
                rect.y = (24 * y) / 10;
                rect.w = 2;
                rect.h = 3;

                pixel = mPalette[mBuffer[(320 * y) + x]];

                SDL_SetRenderDrawColor(SDLrenderer, (pixel & 0x000000FF) - 0x38,
                                       ((pixel & 0x0000FF00) >> 8) - 0x18,
                                       ((pixel & 0x00FF0000) >> 16) - 0x10, 255);
                SDL_RenderFillRect(SDLrenderer, &rect);
            }
        }

        SDL_RenderPresent(SDLrenderer);

#ifndef __EMSCRIPTEN__
        SDL_Delay(1000 / 60);
#endif
    }

    void CRenderer::clear() {
        SDL_SetRenderDrawColor(SDLrenderer, 0,
                               0,
                               0, 255);
        SDL_RenderFillRect(SDLrenderer, NULL);

    }
}
