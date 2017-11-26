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

    TexturePair skybox;
    std::shared_ptr<odb::NativeBitmap> mBackground;
    std::shared_ptr<odb::NativeTexture> foe;
    std::shared_ptr<odb::NativeTexture> bow;

    vector<TexturePair> CRenderer::mNativeTextures;

    const uint8_t CRenderer::mTransparency = getPaletteEntry(0xFFFF00FF);

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

    unsigned char CRenderer::getPaletteEntry(int origin) {

        if ( !(origin & 0xFF000000) ) {
            return mTransparency;
        }

        unsigned char shade = 0;


        shade += (((((origin & 0x0000FF)      ) << 2) >> 8)) << 6;
        shade += (((((origin & 0x00FF00)  >> 8) << 3) >> 8)) << 3;
        shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

        return shade;
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

        for ( const auto& texture : tilesToLoad ) {

            auto nativeTexture = std::make_shared<NativeTexture >();
            auto rotatedTexture = std::make_shared<NativeTexture>();

            auto width = texture[0]->getWidth();
            auto sourceData = (texture[0])->getPixelData();
            auto destRegularData = nativeTexture->data();
            auto destRotatedData = rotatedTexture->data();
            int ratio = width / NATIVE_TEXTURE_SIZE;

            for ( int y = 0; y < NATIVE_TEXTURE_SIZE; ++y ) {
                for ( int x = 0; x < NATIVE_TEXTURE_SIZE; ++x ) {
                    uint32_t pixel = (texture[0])->getPixelData()[ ( width * (y * ratio ) ) + (x * ratio ) ];
                    auto converted = CRenderer::getPaletteEntry( pixel );
                    nativeTexture->data()[ ( NATIVE_TEXTURE_SIZE * y ) + x ] = converted;
                    rotatedTexture->data()[ ( NATIVE_TEXTURE_SIZE * x ) + y ] = converted;
                }
            }

            CRenderer::mNativeTextures.push_back( std::make_pair(nativeTexture, rotatedTexture ) );
        }

        mBackground = loadPNG("tile.png", fileLoader);

        for ( int y = 0; y < 32; ++y ) {
            for ( int x = 0; x < 32; ++x ) {
                uint32_t pixel = mBackground->getPixelData()[ ( 32 * y ) + x ];
                auto converted = CRenderer::getPaletteEntry( pixel );
                mBackground->getPixelData()[ ( 32 * y ) + x ] = converted;
            }
        }
        {
            auto foeBitmap = loadPNG("MonkAttack0.png", fileLoader);
            foe = std::make_shared<odb::NativeTexture>();

            auto width = foeBitmap->getWidth();
            auto sourceData = foeBitmap->getPixelData();
            auto destRegularData = foe->data();
            int ratio = width / NATIVE_TEXTURE_SIZE;

            for (int y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
                for (int x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
                    uint32_t pixel = foeBitmap->getPixelData()[(width * (y * ratio)) + (x * ratio)];
                    auto converted = CRenderer::getPaletteEntry(pixel);
                    foe->data()[(NATIVE_TEXTURE_SIZE * y) + x] = converted;
                }
            }
        }

        {
            auto bowBitmap = loadPNG("crossbow.png", fileLoader);
            bow = std::make_shared<odb::NativeTexture>();

            auto width = bowBitmap->getWidth();
            auto sourceData = bowBitmap->getPixelData();
            auto destRegularData = foe->data();
            int ratio = width / NATIVE_TEXTURE_SIZE;

            for (int y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
                for (int x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
                    uint32_t pixel = bowBitmap->getPixelData()[(width * (y * ratio)) + (x * ratio)];
                    auto converted = CRenderer::getPaletteEntry(pixel);
                    bow->data()[(NATIVE_TEXTURE_SIZE * y) + x] = converted;
                }
            }
        }

        {
            auto sky = loadPNG("clouds.png", fileLoader);
            auto nativeTexture = std::make_shared<NativeTexture >();
            auto rotatedTexture = std::make_shared<NativeTexture>();

            auto width = sky->getWidth();
            auto sourceData = sky->getPixelData();
            auto destRegularData = nativeTexture->data();
            auto destRotatedData = rotatedTexture->data();
            int ratio = width / NATIVE_TEXTURE_SIZE;

            for ( int y = 0; y < NATIVE_TEXTURE_SIZE; ++y ) {
                for ( int x = 0; x < NATIVE_TEXTURE_SIZE; ++x ) {
                    uint32_t pixel = sky->getPixelData()[ ( width * (y * ratio ) ) + (x * ratio ) ];
                    auto converted = CRenderer::getPaletteEntry( pixel );
                    nativeTexture->data()[ ( NATIVE_TEXTURE_SIZE * y ) + x ] = converted;
                    rotatedTexture->data()[ ( NATIVE_TEXTURE_SIZE * x ) + y ] = converted;
                }
            }

            skybox.first = nativeTexture;
            skybox.second = rotatedTexture;
        }

        return tilesToLoad;
    }


    void CRenderer::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {
        auto mapCamera = current->getPosition();

        if (mCached ) {
            return;
        }

        mCached = true;
        mNeedsToRedraw = true;
        mCameraDirection = current->getDirection();
        mCameraPosition = mapCamera;
        Knights::ElementView view;
#ifndef __DJGPP__
        std::cout << "\n\n" << std::endl;
#endif

        for ( int z = 0; z < 40; ++z ) {
            for ( int x = 0; x < 40; ++x ) {
                Knights::Vec2i v = { x, z };
                mElementsMap[z][x] = map.getElementAt(v);

                auto actor = map.getActorAt( v );
                auto item = map.getItemAt(v);
                mActors[ z ][ x ] = EActorsSnapshotElement::kNothing;
                mItems[ z ][ x ] = EItemsSnapshotElement::kNothing;

                if ( actor != nullptr ) {
                    if (actor != current) {
                        mActors[ z ][ x ] = EActorsSnapshotElement::kFallenAttacking1;
                    }
                    view = actor->getView();

                } else if ( item != nullptr ) {
                    mItems[ z ][ x ] = EItemsSnapshotElement ::kCrossbow;
                    view = item->getView();
                } else {
                    view = map.getElementAt(v);
                }
#ifndef __DJGPP__
                std::cout << view;
#endif
            }
#ifndef __DJGPP__
            std::cout << std::endl;
#endif
        }
#ifndef __DJGPP__
        std::cout << std::endl;
#endif
    }

    Knights::CommandType CRenderer::getInput() {
        auto toReturn = mBufferedCommand;
        mBufferedCommand = '.';
        return toReturn;
    }

    Vec2 CRenderer::project(const Vec3&  p ) {
        const static FixP halfWidth{HALF_XRES};
        const static FixP halfHeight{HALF_YRES};
        FixP oneOver = divide( halfHeight, p.mZ );

        return {
                halfWidth + multiply(p.mX, oneOver),
                halfHeight - multiply(p.mY, oneOver)
        };
    }

    void CRenderer::projectAllVertices() {
        const static FixP halfWidth{HALF_XRES};
        const static FixP halfHeight{HALF_YRES};
        const static FixP two{2};

        for ( auto& vertex : mVertices ) {

            if (vertex.first.mZ == 0 ) {
                continue;
            }

            FixP oneOver = divide( halfHeight, divide(vertex.first.mZ, two) );

            vertex.second.mX = halfWidth + multiply(vertex.first.mX, oneOver);
            vertex.second.mY = halfHeight - multiply(vertex.first.mY, oneOver);
        }
    }

    void CRenderer::drawCubeAt(const Vec3& center, TexturePair texture) {

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
                      0, 0,
                      texture, one);
        }


        if (static_cast<int>(center.mY) >= 0 ) {
            drawFloor(ulz1.mY, urz0.mY,
                      ulz1.mX, urz1.mX,
                      ulz0.mX, urz0.mX,
                      0, 0,
                      texture);
        }

        if (static_cast<int>(center.mY) <= 0 ) {
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      0, 0,
                      texture);
        }

        if (static_cast<int>(center.mX) >= 0 ) {
            drawWall(ulz1.mX, ulz0.mX,
                     ulz1.mY, llz1.mY,
                     urz0.mY, lrz0.mY,
                     0, 0,
                     texture, one);
        }

        drawWall( ulz0.mX, urz0.mX,
                  ulz0.mY, llz0.mY,
                  urz0.mY, lrz0.mY,
                  0, 0,
                  texture, one );
    }


    void CRenderer::drawBillboardAt(const Vec3 &center, std::shared_ptr<odb::NativeTexture> texture ) {
        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        const static FixP one{ 1 };
        const static FixP two{ 2 };
        auto textureScale = one / two;
        auto scaledCenter = Vec3{ center.mX, multiply(center.mY, one), center.mZ };

        mVertices[ 0 ].first = ( scaledCenter + Vec3{ -one,  two, 0 });
        mVertices[ 1 ].first = ( scaledCenter + Vec3{  one,  two, 0 });
        mVertices[ 2 ].first = ( scaledCenter + Vec3{ -one, 0, 0 });
        mVertices[ 3 ].first = ( scaledCenter + Vec3{  one, 0, 0 });

        projectAllVertices();

        auto ulz0 = mVertices[0].second;
        auto urz0 = mVertices[1].second;
        auto llz0 = mVertices[2].second;
        auto lrz0 = mVertices[3].second;

        if (kShouldDrawTextures) {
                drawFrontWall( ulz0.mX, ulz0.mY,
                               lrz0.mX, lrz0.mY,
                               mVertices[ 2 ].first.mZ,
                               texture, (textureScale *  two), true );
        }


        if (kShouldDrawOutline) {
            drawLine( ulz0, urz0 );
            drawLine( ulz0, llz0 );
            drawLine( urz0, lrz0 );
            drawLine( llz0, lrz0 );
        }
    }

    void CRenderer::drawColumnAt(const Vec3 &center, const FixP &scale, TexturePair texture, bool mask[3],bool enableAlpha) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        const static FixP one{ 1 };
        const static FixP two{ 2 };

        auto halfScale = scale;
        auto textureScale = halfScale / two;
        auto scaledCenter = Vec3{ center.mX, multiply(center.mY, one), center.mZ };

        //    |\4            /|5
        //    | \  center   / |
        //    |  \    *    /  |
        //    |   \0__|__1/   |7
        //    |6  |   |  |   /
        //     \  |   X  |  /
        //      \ |      | /
        //       \|2____3|/
        mVertices[ 0 ].first = ( scaledCenter + Vec3{ -one,  halfScale, -one });
        mVertices[ 1 ].first = ( scaledCenter + Vec3{  one,  halfScale, -one });
        mVertices[ 2 ].first = ( scaledCenter + Vec3{ -one, -halfScale, -one });
        mVertices[ 3 ].first = ( scaledCenter + Vec3{  one, -halfScale, -one });
        mVertices[ 4 ].first = ( scaledCenter + Vec3{ -one,  halfScale,  one });
        mVertices[ 5 ].first = ( scaledCenter + Vec3{  one,  halfScale,  one });
        mVertices[ 6 ].first = ( scaledCenter + Vec3{ -one, -halfScale,  one });
        mVertices[ 7 ].first = ( scaledCenter + Vec3{  one, -halfScale,  one });

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
            if ( mask[0] && static_cast<int>(center.mX) <= 0 ) {
                drawWall( urz0.mX, urz1.mX,
                          urz0.mY, lrz0.mY,
                          urz1.mY, lrz1.mY,
                          mVertices[ 4 ].first.mZ, mVertices[ 0 ].first.mZ,
                          texture, textureScale);
            }

            if ( mask[2] && static_cast<int>(center.mX) >= 0 ) {
                drawWall(ulz1.mX, ulz0.mX,
                         ulz1.mY, llz1.mY,
                         urz0.mY, lrz0.mY,
                         mVertices[ 5 ].first.mZ, mVertices[ 1 ].first.mZ,
                         texture, textureScale);
            }

            if ( mask[ 1 ] ) {


                std::shared_ptr<odb::NativeTexture > front = texture.first;
                drawFrontWall( ulz0.mX, ulz0.mY,
                               lrz0.mX, lrz0.mY,
                               mVertices[ 2 ].first.mZ,
                               front, (textureScale *  two), enableAlpha );
            }
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

    void CRenderer::drawFloorAt(const Vec3& center, TexturePair texture) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        const static FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one,  0, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one,  0, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  0,  one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  0,  one });

        projectAllVertices();

        auto llz0 = mVertices[0].second;
        auto lrz0 = mVertices[1].second;
        auto llz1 = mVertices[2].second;
        auto lrz1 = mVertices[3].second;

        if ( kShouldDrawTextures ) {
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      mVertices[ 0 ].first.mZ, mVertices[ 2 ].first.mZ,
                      texture);
        }



        if ( kShouldDrawOutline) {
            drawLine( llz0, lrz0 );
            drawLine( llz0, llz1 );
            drawLine( lrz0, lrz1 );
            drawLine( llz1, lrz1 );
        }
    }

    void CRenderer::drawCeilingAt(const Vec3& center, TexturePair texture) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        const static FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one,  0, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one,  0, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  0,  one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  0,  one });

        projectAllVertices();

        auto llz0 = mVertices[0].second;
        auto lrz0 = mVertices[1].second;
        auto llz1 = mVertices[2].second;
        auto lrz1 = mVertices[3].second;

        if (kShouldDrawTextures){
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      center.mZ + one, center.mZ - one,
                      texture);
        }

        if (kShouldDrawOutline) {
            drawLine( llz0, lrz0 );
            drawLine( llz0, llz1 );
            drawLine( lrz0, lrz1 );
            drawLine( llz1, lrz1 );
        }

    }

    void CRenderer::drawLeftNear(const Vec3& center, const FixP &scale, TexturePair texture) {

        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        const static FixP one{ 1 };
        const static FixP two{ 2 };
        auto halfScale = scale;
        auto textureScale = halfScale;
        FixP depth{1};

        if (mCameraDirection == Knights::EDirection::kWest || mCameraDirection == Knights::EDirection::kEast ) {
            depth = FixP{-1};
        }

        mVertices[ 0 ].first = ( center + Vec3{ -one,  halfScale, -depth });
        mVertices[ 1 ].first = ( center + Vec3{  one,  halfScale, depth });
        mVertices[ 2 ].first = ( center + Vec3{ -one, -halfScale, -depth });
        mVertices[ 3 ].first = ( center + Vec3{  one, -halfScale, depth });

        projectAllVertices();

        auto ulz0 = mVertices[0].second;
        auto urz0 = mVertices[1].second;
        auto llz0 = mVertices[2].second;
        auto lrz0 = mVertices[3].second;

        if (kShouldDrawTextures) {
            drawWall( ulz0.mX, urz0.mX,
                      ulz0.mY, llz0.mY,
                      urz0.mY, lrz0.mY,
                      center.mZ - one, center.mZ + one,
                      texture, textureScale );
        }

        if (kShouldDrawOutline){
            drawLine( ulz0, urz0 );
            drawLine( llz0, ulz0 );
            drawLine( llz0, lrz0 );
            drawLine( urz0, lrz0 );
        }
    }


    void CRenderer::drawRightNear(const Vec3& center, const FixP &scale, TexturePair texture) {
        if (static_cast<int>(center.mZ) <= 2 ) {
            return;
        }

        const static FixP one{ 1 };
        const static FixP two{ 2 };
        auto halfScale = scale;
        auto textureScale = halfScale ;
        FixP depth{1};

        if (mCameraDirection == Knights::EDirection::kWest || mCameraDirection == Knights::EDirection::kEast ) {
            depth = FixP{-1};
        }

        mVertices[ 0 ].first = ( center + Vec3{ -one,  halfScale, depth });
        mVertices[ 1 ].first = ( center + Vec3{  one,  halfScale, -depth });
        mVertices[ 2 ].first = ( center + Vec3{ -one, -halfScale, depth });
        mVertices[ 3 ].first = ( center + Vec3{  one, -halfScale, -depth });

        projectAllVertices();

        auto ulz0 = mVertices[0].second;
        auto urz0 = mVertices[1].second;
        auto llz0 = mVertices[2].second;
        auto lrz0 = mVertices[3].second;

        if (kShouldDrawTextures) {
            drawWall( ulz0.mX, urz0.mX,
                      ulz0.mY, llz0.mY,
                      urz0.mY, lrz0.mY,
                      center.mZ - one, center.mZ + one,
                      texture, textureScale );
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
    void CRenderer::drawWall( FixP x0, FixP x1, FixP x0y0, FixP x0y1, FixP x1y0, FixP x1y1, FixP z0, FixP z1, TexturePair texture, FixP textureScaleY ) {

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

        FixP dX = FixP{limit - x};
        FixP upperDyDx = upperDy / dX;
        FixP lowerDyDx = lowerDy / dX;
        FixP dZ = ( z1 - z0 );
        FixP dzDx = dZ / dX;
        FixP z = z0;
        uint8_t pixel = 0;

        FixP u{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        //we can use this statically, since the textures are already loaded.
        //we don't need to fetch that data on every run.
        uint8_t * data = texture.second->data();
        int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        FixP textureSize{ textureWidth };

        FixP du = textureSize / (dX);
        auto ix = x;
        uint8_t * bufferData = getBufferData();
        auto zBuffer = mDepthBuffer.data();

        for (; ix < limit; ++ix ) {
            if ( ix >= 0 && ix < XRES ) {

                FixP diffY = (y1 - y0) / textureScaleY;

                if (diffY == 0) {
                    continue;
                }

                FixP dv = textureSize / diffY;
                FixP v{0};
                auto iu = static_cast<uint8_t >(u);
                auto iz = static_cast<int16_t >(z);
                auto iY0 = static_cast<int16_t >(y0);
                auto iY1 = static_cast<int16_t >(y1);
                auto sourceLineStart = data + (iu * textureWidth);
                auto lineOffset = sourceLineStart;
                auto destinationLine = bufferData + (320 * iY0) + ix;
                auto zBufferLineStart = zBuffer + (320 * iY0) + ix;
                lastV = 0;
                pixel = *(lineOffset);

                for (auto iy = iY0; iy < iY1; ++iy) {

                    if (iy < YRES && iy >= 0 ) {
                        auto iv = static_cast<uint8_t >(v);

                        if (iv != lastV || iu != lastU) {
                            pixel = *(lineOffset);
                        }
                        lineOffset = ((iv % textureWidth) + sourceLineStart);
                        lastU = iu;
                        lastV = iv;
                        if (pixel != mTransparency && ( *zBufferLineStart ) >= iz ) {
                            *zBufferLineStart = iz;
                            *(destinationLine) = pixel;
                        }
                    }
                    destinationLine += (320);
                    zBufferLineStart += 320;
                    v += dv;
                }
            }
            y0 += upperDyDx;
            y1 += lowerDyDx;
            u += du;
            z += dzDx;
        }

    }

    uint8_t * CRenderer::getBufferData() {
        return &mBuffer[0];
    }

    void CRenderer::drawFrontWall( FixP x0, FixP y0, FixP x1, FixP y1, FixP z, std::shared_ptr<odb::NativeTexture> texture, FixP textureScaleY, bool enableAlpha) {
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

        FixP dY = (y1 - y0) / textureScaleY;

        uint8_t pixel = 0 ;

        FixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);

        uint8_t* data = texture->data();
        int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        FixP textureSize{ textureWidth };

        FixP dv = textureSize / (dY);

        auto diffX = ( x1 - x0 );
        auto zBuffer = mDepthBuffer.data();
        auto iX0 = static_cast<int16_t >(x0);
        auto iX1 = static_cast<int16_t >(x1);
        auto iz = static_cast<int16_t >(z);

        if ( iX0 == iX1 ) {
            return;
        }

        FixP du = textureSize / diffX;

        uint8_t * bufferData = getBufferData();

        for (; iy < limit; ++iy ) {
            if (iy < YRES && iy >= 0) {
                FixP u{0};
                auto iv = static_cast<uint8_t >(v);
                auto sourceLineStart = data + ((iv % textureWidth) * textureWidth);
                auto destinationLine = bufferData + (320 * iy) + iX0;
                auto zBufferLineStart = zBuffer + (320 * iy) + iX0;

                lastU = 0;

//                if ( iv == lastV && !enableAlpha ) {
//                    v += dv;
//                    destinationLine = bufferData + (320 * iy);
//                    sourceLineStart = destinationLine - 320;
//
//                    auto start = std::max<int16_t >( 0, iX0 );
//                    auto finish = std::min<int16_t >( (XRES - 1), iX1 );
//                    std::copy( (sourceLineStart + start ), (sourceLineStart + finish), destinationLine + start);
//                    continue;
//                }

                pixel = *(sourceLineStart);

                for (auto ix = iX0; ix < iX1; ++ix) {

                    if (ix < XRES && ix >= 0 ) {
                        auto iu = static_cast<uint8_t >(u);

                        //only fetch the next texel if we really changed the u, v coordinates
                        //(otherwise, would fetch the same thing anyway)
                        if (iv != lastV || iu != lastU) {
                            pixel = *(sourceLineStart );
                        }

                        sourceLineStart += ( iu - lastU);
                        lastU = iu;
                        lastV = iv;
                        if (pixel != mTransparency && ( *zBufferLineStart ) >= iz ) {
                            *zBufferLineStart = iz;
                            *(destinationLine) = pixel;
                        }
                    }
                    ++destinationLine;
                    ++zBufferLineStart;
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
    void CRenderer::drawFloor(FixP y0, FixP y1, FixP x0y0, FixP x1y0, FixP x0y1, FixP x1y1, FixP z0, FixP z1, TexturePair texture ) {

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

        uint8_t pixel = 0 ;

        FixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);
        auto zBuffer = mDepthBuffer.data();
        uint8_t * bufferData = getBufferData();
        uint8_t * data = texture.first->data();
        int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        FixP textureSize{ textureWidth };

        FixP dv = textureSize / (dY);
        FixP dZ = ( z1 - z0 );
        FixP dzDy = dZ / dY;
        FixP z = z0;

        for (; iy < limit; ++iy ) {

            if ( iy < YRES && iy >= 0 ) {

                auto diffX = (x1 - x0);

                if (diffX == 0) {
                    continue;
                }

                auto iX0 = static_cast<int16_t >(x0);
                auto iX1 = static_cast<int16_t >(x1);
                auto iz = static_cast<int16_t >(z);

                FixP du = textureSize / diffX;
                FixP u{0};
                auto iv = static_cast<uint8_t >(v);
                auto sourceLineStart = data + (iv * textureWidth);
                auto destinationLine = bufferData + (320 * iy) + iX0;
                auto zBufferLineStart = zBuffer + (320 * iy) + iX0;
                lastU = 0;
                pixel = *(sourceLineStart);

                for (auto ix = iX0; ix < iX1; ++ix) {

                    if (ix >= 0 && ix < XRES) {
                        auto iu = static_cast<uint8_t >(u);

                        //only fetch the next texel if we really changed the u, v coordinates
                        //(otherwise, would fetch the same thing anyway)
                        if (iv != lastV || iu != lastU) {
                            pixel = *(sourceLineStart);
                        }
                        sourceLineStart += ( iu - lastU );
                        lastU = iu;
                        lastV = iv;

                        if (pixel != mTransparency && ( *zBufferLineStart ) >= iz ) {
                            *zBufferLineStart = iz;
                            *(destinationLine) = pixel;
                        }
                    }
                    ++destinationLine;
                    ++zBufferLineStart;
                    u += du;
                }
            }

            x0 += leftDxDy;
            x1 += rightDxDy;
            v += dv;
            z += dzDy;
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
        const static FixP one{1};

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

            clear();
            std::fill( std::begin(mDepthBuffer), std::end(mDepthBuffer), FixP{255} );

            drawFloor( FixP{0}, FixP{HALF_YRES}, FixP{ -64}, FixP{ XRES + 64},FixP{0}, FixP{XRES}, FixP{254}, FixP{250}, skybox);
            bool facesMask[3];
            EActorsSnapshotElement actorsSnapshotElement = EActorsSnapshotElement::kNothing;
            EItemsSnapshotElement itemsSnapshotElement = EItemsSnapshotElement::kNothing;

            for (int z = 0; z <40; ++z ) {
                for ( int x = 0; x < 40; ++x ) {
                    facesMask[ 0 ] = true;
                    facesMask[ 1 ] = true;
                    facesMask[ 2 ] = true;

                    char element;
                    Vec3 position;
                    switch (mCameraDirection) {
                        case Knights::EDirection::kNorth:

                            element = mElementsMap[z][39 - x];
                            actorsSnapshotElement = mActors[z][39 - x ];
                            itemsSnapshotElement = mItems[z][39 - x ];

                            mCamera = Vec3{ FixP{ 78 - ( 2 * mCameraPosition.x ) },
                                            FixP{-1},
                                            FixP{ ( 2 * mCameraPosition.y ) - 79} };

                            position = mCamera + Vec3{ FixP{- 2 * x}, FixP{ 0 }, FixP{ 80 - ( 2 * z)}};

                            //remember, bounds - 1!
                            facesMask[ 0 ] = !( x > 0 && mElementsMap[z][39 - (x - 1)] == element);
                            facesMask[ 2 ] = !( x < 39 && mElementsMap[z][39 - (x + 1)] == element);
                            facesMask[ 1 ] = !( z < 40 && mElementsMap[z + 1][39 - x] == element);
                            break;

                        case Knights::EDirection::kSouth:
                            element = mElementsMap[39 - z][x];
                            actorsSnapshotElement = mActors[39 - z][x];
                            itemsSnapshotElement = mItems[39 - z][ x ];

                            mCamera = Vec3{ FixP{ ( 2 * mCameraPosition.x ) },
                                            FixP{-1},
                                            FixP{ ( 2 * (39 - mCameraPosition.y) ) - 79} };

                            position = mCamera + Vec3{ FixP{- 2 * x}, FixP{ 0 }, FixP{ 80 - ( 2 * z)}};
                            //remember, bounds - 1!
                            facesMask[ 0 ] = !( x > 0 && mElementsMap[39 - z][(x - 1)] == element);
                            facesMask[ 2 ] = !( x < 39 && mElementsMap[39 - z][(x + 1)] == element);
                            facesMask[ 1 ] = !( z < 40 && mElementsMap[39 - (z + 1)][x] == element);

                            break;
                        case Knights::EDirection::kWest:
                            element = mElementsMap[x][39 - z];
                            itemsSnapshotElement = mItems[x][39 - z ];
                            actorsSnapshotElement = mActors[x][39 - z ];

                            mCamera = Vec3{
                                            FixP{ ( 2 * mCameraPosition.y ) },
                                            FixP{-1},
                                            FixP{ ( 2 * mCameraPosition.x ) }
                            };


                            position = mCamera + Vec3{ FixP{- 2 * x}, FixP{ 0 }, FixP{ ( 2 * z) - 76}};
                            //remember, bounds - 1!
                            facesMask[ 0 ] = !( x > 0 && mElementsMap[x - 1][39 - z] == element);
                            facesMask[ 2 ] = !( x < 39 && mElementsMap[x + 1][39 - z] == element);
                            facesMask[ 1 ] = !( z > 0 && mElementsMap[x][39 - (z - 1)] == element);

                            break;

                            case Knights::EDirection::kEast:
                                element = mElementsMap[x][z];
                                actorsSnapshotElement = mActors[x][z ];
                                itemsSnapshotElement = mItems[x][z ];

                            mCamera = Vec3{
                                    FixP{ - ( 2 * mCameraPosition.y ) },
                                    FixP{-1},
                                    FixP{ 80 - ( 2 * mCameraPosition.x ) }
                            };

                            position = mCamera + Vec3{ FixP{- 2 * ( - x)}, FixP{ 0 }, FixP{ ( 2 * z) - 78}};
                            //remember, bounds - 1!
                            facesMask[ 2 ] = !( x > 0 && mElementsMap[x - 1][z] == element);
                            facesMask[ 0 ] = !( x < 39 && mElementsMap[x + 1][z] == element);
                            facesMask[ 1 ] = !( z < 39 && mElementsMap[x][(z - 1)] == element);
                            break;

                    }

                    auto tileProp = mTileProperties[element];
                    auto heightDiff = tileProp.mCeilingHeight - tileProp.mFloorHeight;
                    auto twiceHeight = multiply( heightDiff, two );
                    auto halfHeightDiff = heightDiff / two;

                    if ( tileProp.mFloorRepeatedTextureIndex > 0 && tileProp.mFloorRepetitions > 0) {

                        switch (tileProp.mGeometryType ) {
                            case kRightNearWall:
                                drawRightNear(
                                        position + Vec3{0, multiply(tileProp.mFloorHeight, two) - tileProp.mFloorRepetitions, 0},
                                        tileProp.mFloorRepetitions,
                                        mNativeTextures[tileProp.mFloorRepeatedTextureIndex]);

                                break;

                            case kLeftNearWall:
                                drawLeftNear(
                                        position + Vec3{0, multiply(tileProp.mFloorHeight, two) - tileProp.mFloorRepetitions, 0},
                                        tileProp.mFloorRepetitions,
                                        mNativeTextures[tileProp.mFloorRepeatedTextureIndex]);
                                break;

                            case kCube:
                            default:
                                drawColumnAt(
                                        position + Vec3{0, multiply(tileProp.mFloorHeight, two) - tileProp.mFloorRepetitions, 0},
                                        tileProp.mFloorRepetitions,
                                        mNativeTextures[tileProp.mFloorRepeatedTextureIndex],
                                        facesMask);
                                break;
                        }
                    }

                    if ( tileProp.mCeilingRepeatedTextureIndex > 0 && tileProp.mCeilingRepetitions > 0 ) {

                        switch (tileProp.mGeometryType ) {
                            case kRightNearWall:
                                drawRightNear(
                                        position + Vec3{0, multiply(tileProp.mCeilingHeight, two) + tileProp.mCeilingRepetitions, 0},
                                        tileProp.mCeilingRepetitions,
                                        mNativeTextures[tileProp.mCeilingRepeatedTextureIndex]);                                break;

                            case kLeftNearWall:
                                drawLeftNear(
                                        position + Vec3{0, multiply(tileProp.mCeilingHeight, two) + tileProp.mCeilingRepetitions, 0},
                                        tileProp.mCeilingRepetitions,
                                        mNativeTextures[tileProp.mCeilingRepeatedTextureIndex]);                                break;

                            case kCube:
                            default:
                                drawColumnAt(
                                        position + Vec3{0, multiply(tileProp.mCeilingHeight, two) + tileProp.mCeilingRepetitions, 0},
                                        tileProp.mCeilingRepetitions,
                                        mNativeTextures[tileProp.mCeilingRepeatedTextureIndex],
                                        facesMask);
                                break;
                        }
                    }

                    if ( tileProp.mFloorTextureIndex != -1 ) {
                        drawFloorAt( position + Vec3{ 0, multiply(tileProp.mFloorHeight, two), 0}, mNativeTextures[ tileProp.mFloorTextureIndex ] );
                    }

                    if ( tileProp.mCeilingTextureIndex != -1 ) {
                        drawCeilingAt(position + Vec3{ 0, multiply(tileProp.mCeilingHeight, two), 0}, mNativeTextures[ tileProp.mCeilingTextureIndex ] );
                    }

                    if ( tileProp.mMainWallTextureIndex > 0 ) {

                        auto scale = tileProp.mCeilingHeight - tileProp.mFloorHeight;

                        switch (tileProp.mGeometryType ) {
                            case kRightNearWall:
                                drawRightNear(
                                        position + Vec3{ 0, multiply( tileProp.mFloorHeight, two) + heightDiff, 0},
                                        heightDiff,
                                        mNativeTextures[ tileProp.mMainWallTextureIndex ] );
                                break;

                            case kLeftNearWall:
                                drawLeftNear(
                                        position + Vec3{ 0, multiply( tileProp.mFloorHeight, two) + heightDiff, 0},
                                        heightDiff,
                                        mNativeTextures[ tileProp.mMainWallTextureIndex ] );
                                break;

                            case kCube:
                            default:
                                drawColumnAt(
                                        position + Vec3{ 0, multiply( tileProp.mFloorHeight, two) + heightDiff, 0},
                                        heightDiff,
                                        mNativeTextures[ tileProp.mMainWallTextureIndex ], facesMask, tileProp.mNeedsAlphaTest );
                                break;
                        }
                    }


                    if ( actorsSnapshotElement != EActorsSnapshotElement::kNothing ) {
                        drawBillboardAt(
                                position + Vec3{ 0, multiply( tileProp.mFloorHeight, two) + heightDiff, 0},
                                foe );
                    }


                    if ( itemsSnapshotElement != EItemsSnapshotElement ::kNothing ) {
                        drawBillboardAt(
                                position + Vec3{ 0, multiply( tileProp.mFloorHeight, two) + heightDiff, 0},
                                bow );
                    }


                }
            }
        }


        for ( int c = 0; c < 8; ++c ) {
            drawSprite( c * 32, 128, mBackground );
        }

        for ( int c = 0; c < (192/32); ++c ) {
            drawSprite( 320 - 32, c * 32, mBackground );
            drawSprite( 320 - 64, c * 32, mBackground );
        }

        const static auto black = 0;
        flip();
    }

    void CRenderer::fill( int x, int y, int dx, int dy, uint8_t pixel ) {
        auto destination = getBufferData();

        for ( int py = 0; py < dy; ++py ) {
            auto destinationLineStart = destination + ( 320 * (y + py) ) + x;
            for ( int px = 0; px < dx; ++px ) {
                (*destinationLineStart) = pixel;
                ++destinationLineStart;
            }
        }
    }

    void CRenderer::drawSprite( int dx, int dy, std::shared_ptr<odb::NativeBitmap> tile ) {
        auto destination = getBufferData();
        auto sourceLine = tile->getPixelData();

        for ( int y = 0; y < 32; ++y ) {
            auto destinationLineStart = destination + ( 320 * (dy + y ) ) + dx;
            auto sourceLineStart = sourceLine + ( 32 * y );
            for ( int x = 0; x < 32; ++x ) {
                (*destinationLineStart) = (*sourceLineStart);
                ++destinationLineStart;
                ++sourceLineStart;
            }
        }
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
