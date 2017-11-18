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
#include <chrono>

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

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <cmath>
#include "NativeBitmap.h"
#include "LoadPNG.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

namespace odb {

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
        SDL_Init( SDL_INIT_EVERYTHING );
        video = SDL_SetVideoMode( 320, 200, 32, 0 );

        for ( int r = 0; r < 256; ++r ) {
            for ( int g = 0; g < 256; ++g ) {
                for ( int b = 0; b < 256; ++b ) {
                    auto pixel = ( r << 16 ) + ( g << 8 ) + ( b );
                    auto paletteEntry = getPaletteEntry( pixel );
                    mPalette[ paletteEntry ] = pixel;
                }
            }
        }

#ifdef __EMSCRIPTEN__
        enterFullScreenMode();
#endif
    }

    void CRenderer::sleep(long ms) {
#ifndef __EMSCRIPTEN__
        SDL_Delay(33);
#endif
    }

    void CRenderer::handleSystemEvents() {
        SDL_Event event;
        const static FixP delta{2};

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
#ifndef __EMSCRIPTEN__
                exit(0);
#endif
            }

            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_q:
#ifndef __EMSCRIPTEN__
                        exit(0);
#endif
                    default:
                        break;
                }
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        exit(0);
                        mNeedsToRedraw = true;
                        break;

                    case SDLK_LEFT:
                        mSpeed.mX += delta;
                        mNeedsToRedraw = true;
                        break;
                    case SDLK_RIGHT:
                        mSpeed.mX -= delta;
                        mNeedsToRedraw = true;
                        break;
                    case SDLK_UP:
                        mSpeed.mY -= delta;
                        mNeedsToRedraw = true;
                        break;
                    case SDLK_DOWN:
                        mSpeed.mY += delta;
                        mNeedsToRedraw = true;
                        break;

                    case SDLK_z:
                        mSpeed.mZ += delta;
                        mNeedsToRedraw = true;
                        break;

                    case SDLK_a:
                        mSpeed.mZ -= delta;
                        mNeedsToRedraw = true;
                        break;

                    default:
                        break;
                }
            }

        }
    }

    void CRenderer::putRaw(int16_t x, int16_t y, uint32_t pixel) {


        if ( x < 0 || x >= 256 || y < 0 || y >= 128 ) {
            return;
        }

        mBuffer[ (320 * y ) + x ] = pixel;
    }

    void CRenderer::flip() {

        for ( int y = 0; y  < 128; ++y ) {
            for ( int x = 0; x < 320; ++x ) {
                SDL_Rect rect;
                rect.x = x;
                rect.y = y;
                rect.w = 1;
                rect.h = 1;
                auto pixel = mPalette[ mBuffer[ (320 * y ) + x ] ];

                SDL_FillRect(video, &rect, SDL_MapRGB(video->format, ((pixel & 0x000000FF)), ((pixel & 0x0000FF00) >> 8),
                                                      ((pixel & 0x00FF0000) >> 16)));

            }
        }

        SDL_Flip(video);
    }

    void CRenderer::clear() {
        auto beginFrame = std::begin( mBuffer );
        auto endFrame = std::end( mBuffer );
        std::fill( beginFrame, endFrame, 0 );
        SDL_FillRect(video, nullptr, 0);
    }
}
