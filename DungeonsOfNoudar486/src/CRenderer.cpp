#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <string>
#include <iostream>
#include <memory>
#include <utility>
#include <map>
#include <functional>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <sstream>
#include <sg14/fixed_point>

using sg14::fixed_point;

#include <EASTL/vector.h>
#include <EASTL/array.h>
#include <NativeBitmap.h>
#include <ETextures.h>
#include <unordered_map>

using eastl::vector;
using eastl::array;
using namespace std::chrono;

#include "IFileLoaderDelegate.h"
#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "LoadPNG.h"


namespace odb {

    const static bool kShouldDrawOutline = false;
    const static bool kShouldDrawTextures = true;

    vector<std::shared_ptr<odb::NativeBitmap>> loadBitmapList(std::string filename, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {
        auto data = fileLoader->loadFileFromPath( filename );
        std::stringstream dataStream;

        dataStream << data;

        std::string buffer;

        vector<std::shared_ptr<odb::NativeBitmap>> toReturn;

        while (dataStream.good()) {
            std::getline(dataStream, buffer);

            toReturn.push_back(loadPNG(buffer, fileLoader));
        }

        return toReturn;
    }

    odb::CTilePropertyMap loadTileProperties( int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {
        std::stringstream roomName("");
        roomName << "tiles";
        roomName << levelNumber;
        roomName << ".prp";
        std::string filename = roomName.str();

        auto data = fileLoader->loadFileFromPath( filename );

        return odb::CTile3DProperties::parsePropertyList( data );
    }


    vector<vector<std::shared_ptr<odb::NativeBitmap>>>
    loadTexturesForLevel(int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {

        std::stringstream roomName("");
        roomName << "tiles";
        roomName << levelNumber;
        roomName << ".lst";
        std::string tilesFilename = roomName.str();
        auto data = fileLoader->loadFileFromPath( tilesFilename );
        std::stringstream dataStream;

        dataStream << data;

        std::string buffer;

        vector<vector<std::shared_ptr<odb::NativeBitmap>>> tilesToLoad;

        while (dataStream.good()) {
            std::getline(dataStream, buffer);
            vector<std::shared_ptr<odb::NativeBitmap>> textures;


            if (buffer.substr(buffer.length() - 4) == ".lst") {
                auto frames = loadBitmapList(buffer, fileLoader );
                for ( const auto frame : frames ) {
                    textures.push_back(frame);
                }
            } else {
                textures.push_back(loadPNG(buffer, fileLoader));
            }

            tilesToLoad.push_back(textures);
        }

        return tilesToLoad;
    }


    void CRenderer::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {
        auto mapCamera = current->getPosition();
//        mCamera = Vec3{ mapCamera.x, 0, mapCamera.y};

        if (mCached ) {
            return;
        }
        mCameraPosition = mapCamera;

        for ( int z = 0; z < 40; ++z ) {
            for ( int x = 0; x < 40; ++x ) {
                mElementsMap[z][x] = map.getElementAt({ x, z });
            }
        }

        if (!mCached ) {
            mCached = true;
            mNeedsToRedraw = true;
        }
    }

    Knights::CommandType CRenderer::getInput() {
        auto toReturn = mBufferedCommand;
        mBufferedCommand = '.';
        return toReturn;
    }

    Vec2 CRenderer::project(const Vec3&  p ) {
        FixP halfWidth{128};
        FixP halfHeight{64};
        FixP oneOver = divide( halfHeight, p.mZ );

        return {
                halfWidth + multiply(p.mX, oneOver),
                halfHeight - multiply(p.mY, oneOver)
        };
    }

    void CRenderer::projectAllVertices() {
        FixP halfWidth{128};
        FixP halfHeight{64};
        FixP two{2};

        for ( auto& vertex : mVertices ) {

            if (vertex.first.mZ == 0 ) {
                continue;
            }

            FixP oneOver = divide( halfHeight, divide(vertex.first.mZ, two) );

            vertex.second.mX = halfWidth + multiply(vertex.first.mX, oneOver);
            vertex.second.mY = halfHeight - multiply(vertex.first.mY, oneOver);
        }
    }

    void CRenderer::drawCubeAt(const Vec3& center, std::shared_ptr<odb::NativeBitmap> texture) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one, -one, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one, -one, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  one, -one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  one, -one });
        mVertices[ 4 ].first = ( center + Vec3{ -one, -one,  one });
        mVertices[ 5 ].first = ( center + Vec3{  one, -one,  one });
        mVertices[ 6 ].first = ( center + Vec3{ -one,  one,  one });
        mVertices[ 7 ].first = ( center + Vec3{  one,  one,  one });

        projectAllVertices();

        auto ulz0 = mVertices[0].second;
        auto urz0 = mVertices[1].second;
        auto llz0 = mVertices[2].second;
        auto lrz0 = mVertices[3].second;
        auto ulz1 = mVertices[4].second;
        auto urz1 = mVertices[5].second;
        auto llz1 = mVertices[6].second;
        auto lrz1 = mVertices[7].second;

        if (static_cast<int>(center.mX) <= 0 ) {
            drawWall( urz0.mX, urz1.mX,
                      urz0.mY, lrz0.mY,
                      urz1.mY, lrz1.mY,
                      texture);
        }


        if (static_cast<int>(center.mY) >= 0 ) {
            drawFloor(ulz1.mY, urz0.mY,
                      ulz1.mX, urz1.mX,
                      ulz0.mX, urz0.mX,
                      texture);
        }

        if (static_cast<int>(center.mY) <= 0 ) {
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      texture);
        }

        if (static_cast<int>(center.mX) >= 0 ) {
            drawWall(ulz1.mX, ulz0.mX,
                     ulz1.mY, llz1.mY,
                     urz0.mY, lrz0.mY,
                     texture);
        }

        drawWall( ulz0.mX, urz0.mX,
                  ulz0.mY, llz0.mY,
                  urz0.mY, lrz0.mY,
                  texture );
    }

    void CRenderer::drawColumnAt(const Vec3 &center, const Vec3 &scale, std::shared_ptr<odb::NativeBitmap> texture) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        FixP one{ 1 };
        FixP two{ 2 };

        auto halfScale = divide(scale.mY, two);

        mVertices[ 0 ].first = ( center + Vec3{ -one,  halfScale + scale.mY, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one,  halfScale + scale.mY, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one, -halfScale, -one });
        mVertices[ 3 ].first = ( center + Vec3{  one, -halfScale, -one });
        mVertices[ 4 ].first = ( center + Vec3{ -one,  halfScale + scale.mY,  one });
        mVertices[ 5 ].first = ( center + Vec3{  one,  halfScale + scale.mY,  one });
        mVertices[ 6 ].first = ( center + Vec3{ -one, -halfScale,  one });
        mVertices[ 7 ].first = ( center + Vec3{  one, -halfScale,  one });

        projectAllVertices();

        auto ulz0 = mVertices[0].second;
        auto urz0 = mVertices[1].second;
        auto llz0 = mVertices[2].second;
        auto lrz0 = mVertices[3].second;
        auto ulz1 = mVertices[4].second;
        auto urz1 = mVertices[5].second;
        auto llz1 = mVertices[6].second;
        auto lrz1 = mVertices[7].second;

        if (kShouldDrawTextures) {
            if (static_cast<int>(center.mX) <= 0 ) {
                drawWall( urz0.mX, urz1.mX,
                          urz0.mY, lrz0.mY,
                          urz1.mY, lrz1.mY,
                          texture);
            }

            if (static_cast<int>(center.mX) >= 0 ) {
                drawWall(ulz1.mX, ulz0.mX,
                         ulz1.mY, llz1.mY,
                         urz0.mY, lrz0.mY,
                         texture);
            }

            drawFrontWall( ulz0.mX, ulz0.mY,
                      lrz0.mX, lrz0.mY,
                      texture );
        }


        if (kShouldDrawOutline) {
            drawLine( ulz0, urz0 );
            drawLine( ulz0, llz0 );
            drawLine( urz0, lrz0 );
            drawLine( llz0, lrz0 );

            drawLine( ulz0, ulz1 );
            drawLine( llz0, llz1 );
            drawLine( ulz1, llz1 );

            drawLine( urz0, urz1 );
            drawLine( lrz0, lrz1 );
            drawLine( urz1, lrz1 );
        }
    }

    void CRenderer::drawFloorAt(const Vec3& center, std::shared_ptr<odb::NativeBitmap> texture) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one,  one, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one,  one, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  one,  one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  one,  one });

        projectAllVertices();

        auto llz0 = mVertices[0].second;
        auto lrz0 = mVertices[1].second;
        auto llz1 = mVertices[2].second;
        auto lrz1 = mVertices[3].second;

        if ( kShouldDrawTextures ) {
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      texture);
        }

        if ( kShouldDrawOutline) {
            drawLine( llz0, lrz0 );
            drawLine( llz0, llz1 );
            drawLine( lrz0, lrz1 );
            drawLine( llz1, lrz1 );
        }
    }

    void CRenderer::drawCeilingAt(const Vec3& center, std::shared_ptr<odb::NativeBitmap> texture) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one,  one, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one,  one, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  one,  one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  one,  one });

        projectAllVertices();

        auto llz0 = mVertices[0].second;
        auto lrz0 = mVertices[1].second;
        auto llz1 = mVertices[2].second;
        auto lrz1 = mVertices[3].second;

        if (kShouldDrawTextures){
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      texture);
        }

        if (kShouldDrawOutline) {
            drawLine( llz0, lrz0 );
            drawLine( llz0, llz1 );
            drawLine( lrz0, lrz1 );
            drawLine( llz1, lrz1 );
        }

    }

    void CRenderer::drawLeftNear(const Vec3& center, const Vec3 &scale, std::shared_ptr<odb::NativeBitmap> texture) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        FixP one{ 1 };
        FixP two{ 2 };
        auto halfScale = divide( scale.mY, two );

        mVertices[ 0 ].first = ( center + Vec3{ -one, halfScale + scale.mY, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one, halfScale + scale.mY, one });
        mVertices[ 2 ].first = ( center + Vec3{ -one, -halfScale, -one });
        mVertices[ 3 ].first = ( center + Vec3{  one, -halfScale, one });

        projectAllVertices();

        auto ulz0 = mVertices[0].second;
        auto urz0 = mVertices[1].second;
        auto llz0 = mVertices[2].second;
        auto lrz0 = mVertices[3].second;

        if (kShouldDrawTextures) {
            drawWall( ulz0.mX, urz0.mX,
                      ulz0.mY, llz0.mY,
                      urz0.mY, lrz0.mY,
                      texture );
        }

        if (kShouldDrawOutline){
            drawLine( ulz0, urz0 );
            drawLine( llz0, ulz0 );
            drawLine( llz0, lrz0 );
            drawLine( urz0, lrz0 );
        }
    }


    void CRenderer::drawRightNear(const Vec3& center, const Vec3 &scale, std::shared_ptr<odb::NativeBitmap> texture) {
        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        FixP one{ 1 };
        FixP two{ 2 };
        auto halfScale = divide( scale.mY, two );

        mVertices[ 0 ].first = ( center + Vec3{ -one, halfScale + scale.mY, one });
        mVertices[ 1 ].first = ( center + Vec3{  one, halfScale + scale.mY, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one, -halfScale, one });
        mVertices[ 3 ].first = ( center + Vec3{  one, -halfScale, -one });

        projectAllVertices();

        auto ulz0 = mVertices[0].second;
        auto urz0 = mVertices[1].second;
        auto llz0 = mVertices[2].second;
        auto lrz0 = mVertices[3].second;

        if (kShouldDrawTextures) {
            drawWall( ulz0.mX, urz0.mX,
                      ulz0.mY, llz0.mY,
                      urz0.mY, lrz0.mY,
                      texture );
        }

        if (kShouldDrawOutline) {
            drawLine( ulz0, urz0 );
            drawLine( llz0, ulz0 );
            drawLine( llz0, lrz0 );
            drawLine( urz0, lrz0 );
        }

    }


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
    void CRenderer::drawWall( FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, std::shared_ptr<odb::NativeBitmap> texture ) {

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

        auto x = static_cast<int16_t >(x0);
        auto limit = static_cast<int16_t >(x1);

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

        FixP upperDy = upperY1 - upperY0;
        FixP lowerDy = lowerY1 - lowerY0;

        FixP y0 = upperY0;
        FixP y1 = lowerY0;

        FixP dX{limit - x};
        FixP upperDyDx = upperDy / dX;
        FixP lowerDyDx = lowerDy / dX;

        uint32_t pixel = 0;

        FixP u{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        //we can use this statically, since the textures are already loaded.
        //we don't need to fetch that data on every run.
        int* data = texture->getPixelData();
        int8_t textureWidth = texture->getWidth();
        FixP textureSize{ textureWidth };

        FixP du = textureSize / (dX);
        auto ix = x;
        int* bufferData = getBufferData();


        for (; ix < limit; ++ix ) {
            if ( ix >= 0 && ix < 256 ) {

                auto diffY = (y1 - y0);

                if (diffY == 0) {
                    continue;
                }

                FixP dv = textureSize / diffY;
                FixP v{0};
                auto iu = static_cast<uint8_t >(u);

                auto iY0 = static_cast<int16_t >(y0);
                auto iY1 = static_cast<int16_t >(y1);
                auto sourceLineStart = data + (iu * textureWidth);
                auto destinationLine = bufferData + (320 * iY0) + ix;
                lastV = 0;
                pixel = *(sourceLineStart);

                for (auto iy = iY0; iy < iY1; ++iy) {

                    if (iy < 128 && iy >= 0 ) {
                        auto iv = static_cast<uint8_t >(v);

                        if (iv != lastV || iu != lastU) {
                            pixel = *(sourceLineStart);
                        }
                        sourceLineStart += (iv - lastV);
                        lastU = iu;
                        lastV = iv;

                        if (pixel & 0xFF000000) {
                            *(destinationLine) = pixel;
                        }
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

    int* CRenderer::getBufferData() {
        return mFrameBuffer->getPixelData();
    }

    void CRenderer::drawFrontWall( FixP x0, FixP y0, FixP x1, FixP y1, std::shared_ptr<odb::NativeBitmap> texture ) {
        //if we have a trapezoid in which the base is smaller
        if ( y0 > y1) {
            //switch y0 with y1
            y0 = y0 + y1;
            y1 = y0 - y1;
            y0 = y0 - y1;
        }

        auto y = static_cast<int16_t >(y0);
        auto limit = static_cast<int16_t >(y1);

        if ( y == limit ) {
            return;
        }

        //what if the trapezoid is flipped horizontally?
        if ( x0 > x1 ) {
            x0 = x0 + x1;
            x1 = x0 - x1;
            x0 = x0 - x1;
        };

        FixP dY = y1 - y0;

        uint32_t pixel = 0 ;

        FixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);

        int* data = texture->getPixelData();
        int8_t textureWidth = texture->getWidth();
        FixP textureSize{ textureWidth };

        FixP dv = textureSize / (dY);

        auto diffX = ( x1 - x0 );

        auto iX0 = static_cast<int16_t >(x0);
        auto iX1 = static_cast<int16_t >(x1);

        if ( iX0 == iX1 ) {
            return;
        }


        FixP du = textureSize / diffX;

        int* bufferData = getBufferData();

        for (; iy < limit; ++iy ) {
            if (iy < 128 && iy >= 0) {
                FixP u{0};
                auto iv = static_cast<uint8_t >(v);
                auto sourceLineStart = data + (iv * textureWidth);
                auto destinationLine = bufferData + (320 * iy) + iX0;

                lastU = 0;

                if ( iv == lastV ) {
                    v += dv;
                    destinationLine = bufferData + (320 * iy);
                    sourceLineStart = destinationLine - 320;

                    auto start = std::max<int16_t >( 0, iX0 );
                    auto finish = std::min<int16_t >( 255, iX1 );
                    std::copy( (sourceLineStart + start ), (sourceLineStart + finish), destinationLine + start);
                    continue;
                }

                pixel = *(sourceLineStart);

                for (auto ix = iX0; ix < iX1; ++ix) {

                    if (ix < 256 && ix >= 0 ) {
                        auto iu = static_cast<uint8_t >(u);

                        //only fetch the next texel if we really changed the u, v coordinates
                        //(otherwise, would fetch the same thing anyway)
                        if (iv != lastV || iu != lastU) {
                            pixel = *(sourceLineStart );
                        }

                        sourceLineStart += ( iu - lastU);
                        lastU = iu;
                        lastV = iv;

                        if (pixel & 0xFF000000) {
                            *(destinationLine) = pixel;
                        }
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
    void CRenderer::drawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, std::shared_ptr<odb::NativeBitmap> texture ) {

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

        auto y = static_cast<int16_t >(y0);
        auto limit = static_cast<int16_t >(y1);

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

        FixP leftDX = lowerX0 - upperX0;
        FixP rightDX = lowerX1 - upperX1;
        FixP dY = y1 - y0;
        FixP leftDxDy = leftDX / dY;
        FixP rightDxDy = rightDX / dY;
        FixP x0 = upperX0;
        FixP x1 = upperX1;

        uint32_t pixel = 0 ;

        FixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);

        int* bufferData = getBufferData();
        int* data = texture->getPixelData();
        int8_t textureWidth = texture->getWidth();
        FixP textureSize{ textureWidth };

        FixP dv = textureSize / (dY);

        for (; iy < limit; ++iy ) {

            if ( iy < 128 && iy >= 0 ) {

                auto diffX = (x1 - x0);

                if (diffX == 0) {
                    continue;
                }

                auto iX0 = static_cast<int16_t >(x0);
                auto iX1 = static_cast<int16_t >(x1);

                FixP du = textureSize / diffX;
                FixP u{0};
                auto iv = static_cast<uint8_t >(v);
                auto sourceLineStart = data + (iv * textureWidth);
                auto destinationLine = bufferData + (320 * iy) + iX0;
                lastU = 0;
                pixel = *(sourceLineStart);

                for (auto ix = iX0; ix < iX1; ++ix) {

                    if (ix >= 0 && ix < 256) {
                        auto iu = static_cast<uint8_t >(u);

                        //only fetch the next texel if we really changed the u, v coordinates
                        //(otherwise, would fetch the same thing anyway)
                        if (iv != lastV || iu != lastU) {
                            pixel = *(sourceLineStart);
                        }
                        sourceLineStart += ( iu - lastU );
                        lastU = iu;
                        lastV = iv;

                        if (pixel & 0xFF000000) {
                            *(destinationLine) = pixel;
                        }
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

    void CRenderer::drawLine(const Vec2& p0, const Vec2& p1) {
        drawLine(static_cast<int16_t >(p0.mX),
                 static_cast<int16_t >(p0.mY),
                 static_cast<int16_t >(p1.mX),
                 static_cast<int16_t >(p1.mY)
        );
    }

    void CRenderer::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {

        if ( x0 == x1 ) {

            int16_t _y0 = y0;
            int16_t _y1 = y1;

            if ( y0 > y1 ) {
                _y0 = y1;
                _y1 = y0;
            }

            for ( int y = _y0; y <= _y1; ++y ) {
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

            for ( int x = _x0; x <= _x1; ++x ) {
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

        for ( int x = x0; x <= x1; ++x ) {
            putRaw( x, static_cast<int16_t >(fy), 0xFFFFFFFF);
            fy += fDeltatY;
        }
    }

    void CRenderer::render(long ms) {

        const static FixP two{2};

        if ( mSpeed.mX ) {
            mSpeed.mX /= two;
            mNeedsToRedraw = true;
        }

        if  ( mSpeed.mY ) {
            mSpeed.mY /= two;
            mNeedsToRedraw = true;
        }

        if  ( mSpeed.mZ ) {
            mSpeed.mZ /= two;
            mNeedsToRedraw = true;
        }

        mCamera += mSpeed;

        if ( mNeedsToRedraw ) {
            mNeedsToRedraw = false;

            if ( clearScr ) {
                clear();
            }

            for (int z = 0; z <40; ++z ) {
                for ( int x = 0; x < 40; ++x ) {

                    auto element = mElementsMap[z][40 - x];
                    auto tileProp = mTileProperties[element];
                    auto heightDiff = tileProp.mCeilingHeight - tileProp.mFloorHeight;
                    auto halfHeightDiff = divide( heightDiff, two );
                    Vec3 position = mCamera + Vec3{ FixP{- 2 * x}, FixP{ -2 }, FixP{2 * (40 - z)}};


                    if ( tileProp.mFloorRepeatedTextureIndex > 0 ) {
                        drawColumnAt(position + Vec3{ 0, tileProp.mFloorHeight - divide(heightDiff, two), 0}, {1, FixP{tileProp.mFloorRepetitions}, 1}, mTextures[ tileProp.mFloorRepeatedTextureIndex ][ 0 ] );
                    }

                    if ( tileProp.mCeilingRepeatedTextureIndex > 0 ) {
                        drawColumnAt(position + Vec3{ 0, tileProp.mCeilingHeight + divide(heightDiff, two), 0}, {1, FixP{tileProp.mCeilingRepetitions}, 1}, mTextures[ tileProp.mCeilingRepeatedTextureIndex ][ 0 ] );
                    }

                    if ( tileProp.mFloorTextureIndex != -1 ) {
                        drawFloorAt( position + Vec3{ 0, tileProp.mFloorHeight, 0}, mTextures[ tileProp.mFloorTextureIndex ][ 0 ] );
                    }

                    if ( tileProp.mCeilingTextureIndex != -1 ) {
                        drawCeilingAt(position + Vec3{ 0, tileProp.mCeilingHeight, 0}, mTextures[ tileProp.mCeilingTextureIndex ][ 0 ] );
                    }


                    if ( tileProp.mMainWallTextureIndex > 0 ) {

                        auto scale = tileProp.mCeilingHeight - tileProp.mFloorHeight;

                        switch (tileProp.mGeometryType ) {
                            case kRightNearWall:
                                drawRightNear(position + Vec3{ 0, halfHeightDiff, 0}, {1, heightDiff, 1}, mTextures[ tileProp.mMainWallTextureIndex ][ 0 ] );
                                break;

                            case kLeftNearWall:
                                drawLeftNear(position + Vec3{ 0, halfHeightDiff, 0}, {1, heightDiff, 1}, mTextures[ tileProp.mMainWallTextureIndex ][ 0 ] );
                                break;

                            case kCube:
                            default:
                                drawColumnAt(position + Vec3{ 0, halfHeightDiff, 0}, {1, heightDiff, 1}, mTextures[ tileProp.mMainWallTextureIndex ][ 0 ] );
                                break;
                        }
                    }


                }
            }


        }

        flip();
    }

    void CRenderer::loadTextures(vector<vector<std::shared_ptr<odb::NativeBitmap>>> textureList, CTilePropertyMap &tile3DProperties) {
        mTextures = textureList;
        mTileProperties = tile3DProperties;

        std::unordered_map<std::string, TextureIndex > textureRegistry;
        textureRegistry["null"] = -1;
        textureRegistry["sky"] = ETextures::Skybox;
        textureRegistry["grass"] = ETextures::Grass;
        textureRegistry["lava"] = ETextures::Lava;
        textureRegistry["floor"] = ETextures::Floor;
        textureRegistry["bricks"] = ETextures::Bricks;
        textureRegistry["arch"] = ETextures::Arch;
        textureRegistry["bars"] = ETextures::Bars;
        textureRegistry["begin"] = ETextures::Begin;
        textureRegistry["exit"] = ETextures::Exit;
        textureRegistry["bricksblood"] = ETextures::BricksBlood;
        textureRegistry["brickscandles"] = ETextures::BricksCandles;
        textureRegistry["stonegrassfar"] = ETextures::StoneGrassFar;
        textureRegistry["grassstonefar"] = ETextures::GrassStoneFar;
        textureRegistry["stonegrassnear"] = ETextures::StoneGrassNear;
        textureRegistry["grassstonenear"] = ETextures::GrassStoneNear;
        textureRegistry["ceiling"] = ETextures::Ceiling;
        textureRegistry["ceilingdoor"] = ETextures::CeilingDoor;
        textureRegistry["ceilingbegin"] = ETextures::CeilingBegin;
        textureRegistry["ceilingend"] = ETextures::CeilingEnd;
        textureRegistry["ceilingbars"] = ETextures::CeilingBars;
        textureRegistry["rope"] = ETextures::Rope;
        textureRegistry["slot"] = ETextures::Slot;
        textureRegistry["magicseal"] = ETextures::MagicSeal;
        textureRegistry["shutdoor"] = ETextures::ShutDoor;
        textureRegistry["cobblestone"] = ETextures::Cobblestone;
        textureRegistry["fence"] = ETextures::Fence;

        for ( auto& propMap : mTileProperties ) {
            auto tileProp = propMap.second;
            tileProp.mCeilingTextureIndex = (textureRegistry[tileProp.mCeilingTexture]);
            tileProp.mFloorTextureIndex = (textureRegistry[tileProp.mFloorTexture]);
            tileProp.mCeilingRepeatedTextureIndex = (textureRegistry[tileProp.mCeilingRepeatedWallTexture]);
            tileProp.mFloorRepeatedTextureIndex = (textureRegistry[tileProp.mFloorRepeatedWallTexture]);
            tileProp.mMainWallTextureIndex = (textureRegistry[tileProp.mMainWallTexture]);
            propMap.second = tileProp;
        }
    }
}
