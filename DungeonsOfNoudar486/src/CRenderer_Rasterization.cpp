#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <string>
#include <memory>
#include <utility>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <chrono>
#include <sg14/fixed_point>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using sg14::fixed_point;
using eastl::vector;
using eastl::array;
using namespace std::chrono;

#include "NativeBitmap.h"
#include "ETextures.h"
#include "IFileLoaderDelegate.h"
#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CStorageItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "MapWithCharKey.h"
#include "CMap.h"
#include "IRenderer.h"
#include "RasterizerCommon.h"

#include "IFileLoaderDelegate.h"
#include "CGame.h"

#include "CRenderer.h"
#include "LoadPNG.h"
#include "VisibilityStrategy.h"

namespace odb {

    const uint8_t CRenderer::mTransparency = getPaletteEntry(0xFFFF00FF);

    /*
     *         /|x1y0
     * x0y0   / |
     *       |  |
     *       |  |
     * x0y1  |  |
     *       \  |
     *        \ |
     *         \| x1y1
     */
    void CRenderer::drawWall( FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, std::shared_ptr<odb::NativeTexture> texture, FixP textureScaleY ) {

        if ( x0 > x1) {
            //switch x0 with x1
            x0 = x0 + x1;
            x1 = x0 - x1;
            x0 = x0 - x1;

            //switch x0y0 with x1y0
            x0y0 = x0y0 + x1y0;
            x1y0 = x0y0 - x1y0;
            x0y0 = x0y0 - x1y0;

            //switch x0y1 with x1y1
            x0y1 = x0y1 + x1y1;
            x1y1 = x0y1 - x1y1;
            x0y1 = x0y1 - x1y1;
        }

        const auto x = static_cast<int16_t >(x0);
        const auto limit = static_cast<int16_t >(x1);

        if ( x == limit ) {
            return;
        }

        FixP upperY0 = x0y0;
        FixP lowerY0 = x0y1;
        FixP upperY1 = x1y0;
        FixP lowerY1 = x1y1;

        if ( x0y0 > x0y1 ) {
            upperY0 = x0y1;
            lowerY0 = x0y0;
            upperY1 = x1y1;
            lowerY1 = x1y0;
        };

        const FixP upperDy = upperY1 - upperY0;
        const FixP lowerDy = lowerY1 - lowerY0;

        FixP y0 = upperY0;
        FixP y1 = lowerY0;

        const FixP dX = FixP{limit - x};
        const FixP upperDyDx = upperDy / dX;
        const FixP lowerDyDx = lowerDy / dX;

        uint8_t pixel = 0;
        TextureFixP u{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        //we can use this statically, since the textures are already loaded.
        //we don't need to fetch that data on every run.
        const uint8_t * data = texture->data();
        const int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        const FixP textureSize{ textureWidth };

        const TextureFixP du = textureSize / (dX);
        auto ix = x;
        uint8_t * bufferData = getBufferData();

        for (; ix < limit; ++ix ) {
            if ( ix >= 0 && ix < XRES ) {

                const FixP diffY = (y1 - y0) / textureScaleY;

                if (diffY == 0) {
                    continue;
                }

                const TextureFixP dv = textureSize / diffY;
                TextureFixP v{0};
                auto iu = static_cast<uint8_t >(u);
                auto iY0 = static_cast<int16_t >(y0);
                auto iY1 = static_cast<int16_t >(y1);
                auto sourceLineStart = data + (iu * textureWidth);
                auto lineOffset = sourceLineStart;
                auto destinationLine = bufferData + (320 * iY0) + ix;

                lastV = 0;
                pixel = *(lineOffset);

                for (auto iy = iY0; iy < iY1; ++iy) {

                    if (iy < YRES && iy >= 0 ) {
                        const auto iv = static_cast<uint8_t >(v);

                        if (iv != lastV ) {
                            pixel = *(lineOffset);
                            lineOffset = ((iv & (textureWidth - 1)) + sourceLineStart);
                            lastU = iu;
                            lastV = iv;
                        }

                        if (pixel != mTransparency ) {
                                *(destinationLine) = pixel;
                        }

#ifdef SDLSW
                        if ( mSlow ) {
                            flip();
                        }
#endif

                    }
                    destinationLine += (320);
                    v += dv;
                }
            }
            y0 += upperDyDx;
            y1 += lowerDyDx;
            u += du;
        }

    }

    void CRenderer::drawMask( FixP x0, FixP y0, FixP x1, FixP y1 ) {
        //if we have a quad in which the base is smaller
        if ( y0 > y1) {
            //switch y0 with y1
            y0 = y0 + y1;
            y1 = y0 - y1;
            y0 = y0 - y1;
        }

        const auto y = static_cast<int16_t >(y0);
        const auto limit = static_cast<int16_t >(y1);

        if ( y == limit ) {
            //degenerate
            return;
        }

        //what if the quad is flipped horizontally?
        if ( x0 > x1 ) {
            x0 = x0 + x1;
            x1 = x0 - x1;
            x0 = x0 - x1;
        };

        const FixP dY = (y1 - y0);

        uint8_t pixel = 0 ;

        auto iy = static_cast<int16_t >(y);

        const auto diffX = ( x1 - x0 );

        auto iX0 = static_cast<int16_t >(x0);
        auto iX1 = static_cast<int16_t >(x1);

        if ( iX0 == iX1 ) {
            //degenerate case
            return;
        }

        uint8_t * bufferData = getBufferData();

        for (; iy < limit; ++iy ) {
            if (iy < YRES && iy >= 0) {
                auto destinationLine = bufferData + (320 * iy) + iX0;

                for (auto ix = iX0; ix < iX1; ++ix) {

                    if (ix < XRES && ix >= 0 ) {

                        *(destinationLine) = pixel;
#ifdef SDLSW
                        if ( mSlow ) {
                            flip();
                        }
#endif
                    }
                    ++destinationLine;
                }

            }
        }
    }

    void CRenderer::drawFrontWall( FixP x0, FixP y0, FixP x1, FixP y1, std::shared_ptr<odb::NativeTexture> texture, FixP textureScaleY, bool enableAlpha) {
        //if we have a quad in which the base is smaller
        if ( y0 > y1) {
            //switch y0 with y1
            y0 = y0 + y1;
            y1 = y0 - y1;
            y0 = y0 - y1;
        }

        const auto y = static_cast<int16_t >(y0);
        const auto limit = static_cast<int16_t >(y1);

        if ( y == limit ) {
            //degenerate
            return;
        }

        //what if the quad is flipped horizontally?
        if ( x0 > x1 ) {
            x0 = x0 + x1;
            x1 = x0 - x1;
            x0 = x0 - x1;
        };

        const FixP dY = (y1 - y0) / textureScaleY;

        uint8_t pixel = 0 ;

        TextureFixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);

        uint8_t* data = texture->data();
        const int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        const FixP textureSize{ textureWidth };

        const TextureFixP dv = textureSize / (dY);

        const auto diffX = ( x1 - x0 );

        auto iX0 = static_cast<int16_t >(x0);
        auto iX1 = static_cast<int16_t >(x1);

        if ( iX0 == iX1 ) {
            //degenerate case
            return;
        }

        const TextureFixP du = textureSize / diffX;

        uint8_t * bufferData = getBufferData();

        for (; iy < limit; ++iy ) {
            if (iy < YRES && iy >= 0) {
                TextureFixP u{0};
                const auto iv = static_cast<uint8_t >(v);
                auto sourceLineStart = data + ((iv & (textureWidth - 1)) * textureWidth);
                auto destinationLine = bufferData + (320 * iy) + iX0;

                lastU = 0;


                if ( iv == lastV && !enableAlpha ) {
                    v += dv;
                    destinationLine = bufferData + (320 * iy);
                    sourceLineStart = destinationLine - 320;
                    auto start = std::max<int16_t >( 0, iX0 );
                    auto finish = std::min<int16_t >( (XRES - 1), iX1 );
                    std::copy( (sourceLineStart + start ), (sourceLineStart + finish), destinationLine + start);
                    continue;
                }

                pixel = *(sourceLineStart);

                for (auto ix = iX0; ix < iX1; ++ix) {

                    if (ix < XRES && ix >= 0 ) {

                        const auto iu = static_cast<uint8_t >(u);

                        //only fetch the next texel if we really changed the u, v coordinates
                        //(otherwise, would fetch the same thing anyway)
                        if ( iu != lastU) {
                            pixel = *(sourceLineStart );
                            sourceLineStart += ( iu - lastU);
                            lastU = iu;
                            lastV = iv;
                        }

                        if (pixel != mTransparency) {
                            *(destinationLine) = pixel;
                        }
#ifdef SDLSW
                        if ( mSlow ) {
                            flip();
                        }
#endif
                    }
                    ++destinationLine;
                    u += du;
                }

            }
            v += dv;
        }
    }


    /*
     *     x0y0 ____________ x1y0
     *         /            \
     *        /             \
     *  x0y1 /______________\ x1y1
     */
    void CRenderer::drawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, std::shared_ptr<NativeTexture > texture ) {

        //if we have a trapezoid in which the base is smaller
        if ( y0 > y1) {
            //switch y0 with y1
            y0 = y0 + y1;
            y1 = y0 - y1;
            y0 = y0 - y1;

            //switch x0y0 with x0y1
            x0y0 = x0y0 + x0y1;
            x0y1 = x0y0 - x0y1;
            x0y0 = x0y0 - x0y1;

            //switch x1y0 with x1y1
            x1y0 = x1y0 + x1y1;
            x1y1 = x1y0 - x1y1;
            x1y0 = x1y0 - x1y1;
        }

        const auto y = static_cast<int16_t >(y0);
        const auto limit = static_cast<int16_t >(y1);

        if ( y == limit ) {
            return;
        }

        FixP upperX0 = x0y0;
        FixP upperX1 = x1y0;
        FixP lowerX0 = x0y1;
        FixP lowerX1 = x1y1;

        //what if the trapezoid is flipped horizontally?
        if ( x0y0 > x1y0 ) {
            upperX0 = x1y0;
            upperX1 = x0y0;
            lowerX0 = x1y1;
            lowerX1 = x0y1;
        };

        const FixP leftDX = lowerX0 - upperX0;
        const FixP rightDX = lowerX1 - upperX1;
        const FixP dY = y1 - y0;
        const FixP leftDxDy = leftDX / dY;
        const FixP rightDxDy = rightDX / dY;
        FixP x0 = upperX0;
        FixP x1 = upperX1;

        uint8_t pixel = 0 ;

        TextureFixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);

        uint8_t * bufferData = getBufferData();
        uint8_t * data = texture->data();
        const int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        const FixP textureSize{ textureWidth };

        const TextureFixP dv = textureSize / (dY);

        for (; iy < limit; ++iy ) {

            if ( iy < YRES && iy >= 0 ) {

                const auto diffX = (x1 - x0);

                if (diffX == 0) {
                    continue;
                }

                auto iX0 = static_cast<int16_t >(x0);
                auto iX1 = static_cast<int16_t >(x1);

                const TextureFixP du = textureSize / diffX;
                TextureFixP u{0};
                const auto iv = static_cast<uint8_t >(v);
                auto sourceLineStart = data + (iv * textureWidth);
                auto destinationLine = bufferData + (320 * iy) + iX0;
                lastU = 0;
                pixel = *(sourceLineStart);

                for (auto ix = iX0; ix < iX1; ++ix) {

                    if (ix >= 0 && ix < XRES) {
                        const auto iu = static_cast<uint8_t >(u);

                        //only fetch the next texel if we really changed the u, v coordinates
                        //(otherwise, would fetch the same thing anyway)
                        if ( iu != lastU) {
                            pixel = *(sourceLineStart);
                            sourceLineStart += ( iu - lastU );
                            lastU = iu;
                            lastV = iv;
                        }

                        if (pixel != mTransparency ) {
                                *(destinationLine) = pixel;
                        }
#ifdef SDLSW
                        if ( mSlow ) {
                            flip();
                        }
#endif
                    }
                    ++destinationLine;
                    u += du;
                }
            }

            x0 += leftDxDy;
            x1 += rightDxDy;
            v += dv;
        }
    }

    void CRenderer::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {

        if ( x0 == x1 ) {

            int16_t _y0 = y0;
            int16_t _y1 = y1;

            if ( y0 > y1 ) {
                _y0 = y1;
                _y1 = y0;
            }

            for ( auto y = _y0; y <= _y1; ++y ) {
                putRaw( x0, y, 0xFFFFFFFF);
            }
            return;
        }

        if ( y0 == y1 ) {
            int16_t _x0 = x0;
            int16_t _x1 = x1;

            if ( x0 > x1 ) {
                _x0 = x1;
                _x1 = x0;
            }

            for ( auto x = _x0; x <= _x1; ++x ) {
                putRaw( x, y0, 0xFFFFFFFF);
            }
            return;
        }

        //switching x0 with x1
        if( x0 > x1 ) {
            x0 = x0 + x1;
            x1 = x0 - x1;
            x0 = x0 - x1;

            y0 = y0 + y1;
            y1 = y0 - y1;
            y0 = y0 - y1;
        }

        FixP fy = FixP{ y0 };
        FixP fLimitY = FixP{ y1 };
        FixP fDeltatY = FixP{ y1 - y0 } / FixP{ x1 - x0 };

        for ( auto x = x0; x <= x1; ++x ) {
            putRaw( x, static_cast<int16_t >(fy), 0xFFFFFFFF);
            fy += fDeltatY;
        }
    }

    void CRenderer::fill( uint16_t x, uint16_t y, uint16_t dx, uint16_t dy, uint8_t pixel ) {

        if (pixel == mTransparency ) {
            return;
        }

        auto destination = getBufferData();

        for ( auto py = 0; py < dy; ++py ) {
            auto destinationLineStart = destination + ( 320 * (y + py) ) + x;
            std::fill( destinationLineStart, destinationLineStart + dx, pixel );
        }
    }

    void CRenderer::drawSprite( uint16_t dx, uint16_t dy, std::shared_ptr<odb::NativeTexture> tile ) {
        auto destination = getBufferData();
        auto sourceLine = &((*tile)[0]);

        for ( auto y = 0; y < 32; ++y ) {
            auto destinationLineStart = destination + ( 320 * (dy + y ) ) + dx;
            auto sourceLineStart = sourceLine + ( 32 * y );

            for ( auto x = 0; x < 32; ++x ) {
                auto pixel = *sourceLineStart;

                if (pixel != CRenderer::mTransparency ) {
                    *destinationLineStart = pixel;
                }

                ++destinationLineStart;
                ++sourceLineStart;
            }

        }
    }

    void CRenderer::drawBitmap( uint16_t dx, uint16_t dy, std::shared_ptr<odb::NativeBitmap> tile ) {
        auto destination = getBufferData();
        auto sourceLine = tile->getPixelData();
        auto height = tile->getHeight();
        auto width = tile->getWidth();
        for ( auto y = 0; y < height; ++y ) {
            auto destinationLineStart = destination + ( 320 * (dy + y ) ) + dx;
            auto sourceLineStart = sourceLine + ( width * y );
            for ( auto x = 0; x < width; ++x ) {
                *destinationLineStart = getPaletteEntry(*sourceLineStart);
                ++sourceLineStart;
                ++destinationLineStart;
            }
        }
    }

    void CRenderer::drawTextAt( uint16_t x, uint16_t y, const char* text, uint8_t colour ) {
        auto len = strlen(text);
        auto dstX = (x - 1) * 8;
        auto dstY = (y - 1) * 8;
        auto dstBuffer = getBufferData();
        auto fontWidth = mFont->getWidth();
        auto fontPixelData = mFont->getPixelData();

        for ( auto c = 0; c < len; ++c ) {
            auto ascii = text[c] - ' ';
            auto line = ascii >> 5;
            auto col = ascii & 31;
            auto letter = fontPixelData + ( col * 8 ) + ( fontWidth * ( line * 8 ) );

            if ( text[c] == '\n' || dstX >= 320 ) {
                dstX = 0;
                dstY += 8;
                continue;
            }

            if ( text[c] == ' ' ) {
                dstX += 8;
                continue;
            }

            for ( auto srcY = 0; srcY < 8; ++srcY ) {

                auto letterSrc = letter + ( fontWidth * srcY );
                auto letterDst = dstBuffer + dstX + ( 320 * (dstY + srcY ) );

                for (auto srcX = 0; srcX < 8; ++srcX ) {

                    if ((*letterSrc) & 0xFF000000) {
                        *letterDst = colour;
                    }

                    ++letterSrc;
                    ++letterDst;
                }
            }
            dstX += 8;
        }
    }
}
