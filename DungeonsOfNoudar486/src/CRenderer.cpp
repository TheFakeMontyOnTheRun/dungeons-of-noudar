#ifdef SDLSW
#include <iostream>
#endif
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
#include "CMap.h"
#include "IRenderer.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "LoadPNG.h"
#include "VisibilityStrategy.h"

namespace odb {

    const static bool kShouldDrawOutline = false;
    const static bool kShouldDrawTextures = true;
    const static bool kShouldDrawSkybox = false;
    const static bool kShouldDrawBlackMasks = true;
    const static auto kMinZCull = FixP{1};

    std::shared_ptr<odb::NativeTexture> mBackground;
    std::shared_ptr<odb::NativeTexture> foe;
    std::shared_ptr<odb::NativeTexture> rope;
    std::shared_ptr<odb::NativeTexture> monk;
    std::shared_ptr<odb::NativeTexture> weakDemon;
    std::shared_ptr<odb::NativeTexture> strongDemon;
    std::shared_ptr<odb::NativeTexture> cocoon;
    std::shared_ptr<odb::NativeTexture> specter;
    std::shared_ptr<odb::NativeTexture> warthog;
    std::shared_ptr<odb::NativeTexture> bow;
    std::shared_ptr<odb::NativeTexture> sword;
    std::shared_ptr<odb::NativeTexture> shield;
    std::shared_ptr<odb::NativeTexture> token;
    std::shared_ptr<NativeTexture > skybox;

    VisMap visMap;
    IntMap intMap;
    DistanceDistribution distances;
    vector<TexturePair> CRenderer::mNativeTextures;
    const uint8_t CRenderer::mTransparency = getPaletteEntry(0xFFFF00FF);

    vector<std::string> splitLists(const std::string& data) {
        vector<std::string> toReturn;
        auto buffer = std::string(data);
        buffer.push_back('\n');
        int lastPoint = 0;
        int since = 0;
        const auto bufferBegin = std::begin( buffer );
        for (const auto& c : buffer ) {
            ++since;
            if ( c == '\n' ) {
                auto item = std::string( bufferBegin + lastPoint, bufferBegin + lastPoint + since - 1 );
                lastPoint += since;
                if ( !item.empty()) {
                    toReturn.push_back(item);
                }
                since = 0;
            }
        }

        return toReturn;
    }

    vector<std::shared_ptr<odb::NativeBitmap>> loadBitmapList(std::string filename, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {
        vector<std::shared_ptr<odb::NativeBitmap>> toReturn;
        const auto buffer = fileLoader->loadFileFromPath( filename );
        const auto list = splitLists(buffer);
        for (const auto& filename : list ) {
            toReturn.push_back(loadPNG(filename, fileLoader));
        }

        return toReturn;
    }

    odb::CTilePropertyMap loadTileProperties( int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {
        char buffer[64];
        snprintf(buffer, 64, "tiles%d.prp", levelNumber);

        const auto data = fileLoader->loadFileFromPath( buffer );

        return odb::CTile3DProperties::parsePropertyList( data );
    }

    uint8_t CRenderer::getPaletteEntry(uint32_t origin) {

        if ( !(origin & 0xFF000000) ) {
            return mTransparency;
        }

        uint8_t shade = 0;


        shade += (((((origin & 0x0000FF)      ) << 2) >> 8)) << 6;
        shade += (((((origin & 0x00FF00)  >> 8) << 3) >> 8)) << 3;
        shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

        return shade;
    }


    std::shared_ptr<odb::NativeTexture> makeTexture(const std::string& path, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {
        const auto bitmap = loadPNG(path, fileLoader);
        auto texture = std::make_shared<odb::NativeTexture>();

        const auto width = bitmap->getWidth();
        const int ratio = width / NATIVE_TEXTURE_SIZE;

        for (int y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
            for (int x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
                const uint32_t pixel = bitmap->getPixelData()[(width * (y * ratio)) + (x * ratio)];
                const auto converted = CRenderer::getPaletteEntry(pixel);
                texture->data()[(NATIVE_TEXTURE_SIZE * y) + x] = converted;
            }
        }

        return texture;
    }

    TexturePair makeTexturePair( std::shared_ptr<odb::NativeBitmap> bitmap ) {
        auto nativeTexture = std::make_shared<NativeTexture >();
        auto rotatedTexture = std::make_shared<NativeTexture>();

        const auto width = bitmap->getWidth();
        const int ratio = width / NATIVE_TEXTURE_SIZE;

        for ( int y = 0; y < NATIVE_TEXTURE_SIZE; ++y ) {
            for ( int x = 0; x < NATIVE_TEXTURE_SIZE; ++x ) {
                const uint32_t pixel = bitmap->getPixelData()[ ( width * (y * ratio ) ) + (x * ratio ) ];
                const auto converted = CRenderer::getPaletteEntry( pixel );
                nativeTexture->data()[ ( NATIVE_TEXTURE_SIZE * y ) + x ] = converted;
                rotatedTexture->data()[ ( NATIVE_TEXTURE_SIZE * x ) + y ] = converted;
            }
        }

        return std::make_pair(nativeTexture, rotatedTexture );
    }

    vector<vector<std::shared_ptr<odb::NativeBitmap>>>
    loadTexturesForLevel(int levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {
        char tilesFilename[64];
        snprintf(tilesFilename, 64, "tiles%d.lst", levelNumber);
        const auto data = fileLoader->loadFileFromPath( tilesFilename );
        const auto list = splitLists(data);

        vector<vector<std::shared_ptr<odb::NativeBitmap>>> tilesToLoad;

        CRenderer::mNativeTextures.clear();

        for ( const auto& frameList : list ) {
            vector<std::shared_ptr<odb::NativeBitmap>> textures;


            if (frameList.substr(frameList.length() - 4) == ".lst") {
                const auto frames = loadBitmapList(frameList, fileLoader );
                for ( const auto frame : frames ) {
                    textures.push_back(frame);
                }
            } else {
                textures.push_back(loadPNG(frameList, fileLoader));
            }

            CRenderer::mNativeTextures.push_back( makeTexturePair(textures[0]) );
        }

        mBackground = makeTexture("tile.png", fileLoader);
        warthog = makeTexture("WarthogAttack0.png", fileLoader);
        strongDemon = makeTexture("StrongDemonAttack0.png", fileLoader);
        weakDemon = makeTexture("WeakDemonAttack0.png", fileLoader);
        cocoon = makeTexture("CocoonStanding0.png", fileLoader);
        foe = makeTexture("FallenAttack0.png", fileLoader);
        specter = makeTexture("EvilSpiritAttack0.png", fileLoader);
        rope = makeTexture("rope.png", fileLoader);
        monk = makeTexture("MonkAttack0.png", fileLoader);
        bow = makeTexture("crossbow.png", fileLoader);
        token = makeTexture("token.png", fileLoader);
        sword = makeTexture("falcata.png", fileLoader);
        shield = makeTexture("shield.png", fileLoader);

        if ( kShouldDrawSkybox) {
            skybox = makeTexture("clouds.png",                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          fileLoader);
        }

        for ( auto& distanceLine : distances ) {
            distanceLine.reserve(Knights::kMapSize + Knights::kMapSize);
        }


        return tilesToLoad;
    }


    void CRenderer::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {
        const auto mapCamera = current->getPosition();

        if (mCached) {
            return;
        }

        mCached = true;
        mNeedsToRedraw = true;
        mCameraDirection = current->getDirection();
        mCameraPosition = mapCamera;
        Knights::ElementView view;

        mHealth = current->getHP();

        auto item = current->getSelectedItem();

        if (item != nullptr) {
            mItemName = current->getSelectedItem()->to_string();

            if (item->getView() == 'y') {
                mCurrentItem = EItemsSnapshotElement::kCrossbow;
                mItemCapacity = ((Knights::CStorageItem*)(item.get()))->getAmount();
                mUsageCost = 3;
            } else if (item->getView() == '+') {
                mCurrentItem = EItemsSnapshotElement::kCross;
                mUsageCost = 1;
            } else if (item->getView() == 'v') {
                mCurrentItem = EItemsSnapshotElement::kShield;
                mItemCapacity = ((Knights::CStorageItem*)(item.get()))->getAmount();
                mUsageCost = 20;
            } else if (item->getView() == 't') {
                mCurrentItem = EItemsSnapshotElement::kSword;
                mUsageCost = 1;
            }
        } else {
            mCurrentItem = EItemsSnapshotElement::kNothing;
            mItemName = "";
        }


        for (uint8_t z = 0; z < Knights::kMapSize; ++z) {
            for (uint8_t x = 0; x < Knights::kMapSize; ++x) {
                const Knights::Vec2i v = {x, z};
                const auto actor = map.getActorAt(v);
                const auto item = map.getItemAt(v);

                auto elementView = map.getElementAt(v);;

                mElementsMap[z][x] = elementView;
                intMap[z][x] = elementView;
                mActors[z][x] = EActorsSnapshotElement::kNothing;
                mItems[z][x] = EItemsSnapshotElement::kNothing;

                if (actor != nullptr) {
                    if (actor != current) {
                        if ( actor->getView() == '$') {
                            mActors[z][x] = EActorsSnapshotElement::kFallenAttacking0;
                        } else if ( actor->getView() == 'C') {
                            mActors[z][x] = EActorsSnapshotElement::kCocoonStanding0;
                        } else if ( actor->getView() == '@') {
                            mActors[z][x] = EActorsSnapshotElement::kMonkAttacking0;
                        } else if ( actor->getView() == 'J') {
                            mActors[z][x] = EActorsSnapshotElement::kWarthogAttacking0;
                        } else if ( actor->getView() == 'd') {
                            mActors[z][x] = EActorsSnapshotElement::kWeakenedDemonAttacking0;
                        } else if ( actor->getView() == 'D') {
                            mActors[z][x] = EActorsSnapshotElement::kStrongDemonAttacking0;
                        } else if ( actor->getView() == 'w') {
                            mActors[z][x] = EActorsSnapshotElement::kEvilSpiritAttacking0;
                        } else if ( actor->getView() == 'T') {
                            mActors[z][x] = EActorsSnapshotElement::kRope;
                        } else {
                            mActors[z][x] = EActorsSnapshotElement::kFallenAttacking1;
                        }
                    }
                    view = actor->getView();

                } else if (item != nullptr) {
                    if (item->getView() == 'y') {
                        mItems[z][x] = EItemsSnapshotElement::kCrossbow;
                    } else if (item->getView() == '+') {
                        mItems[z][x] = EItemsSnapshotElement::kCross;
                    } else if (item->getView() == 'v') {
                        mItems[z][x] = EItemsSnapshotElement::kShield;
                    }

                    view = item->getView();
                } else {
                    view = map.getElementAt(v);
                }
            }
        }
#ifndef SDLSW
        auto t0 = uclock();
#endif

        VisibilityStrategy::castVisibility(visMap, intMap, mCameraPosition, mCameraDirection, true, distances);
#ifndef SDLSW
        auto t1 = uclock();
        mProcVisTime += (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
#endif
    }

    Knights::CommandType CRenderer::peekInput() {
        return mBufferedCommand;
    }

    Knights::CommandType CRenderer::getInput() {
        const auto toReturn = mBufferedCommand;
        mBufferedCommand = '.';
        return toReturn;
    }

    void CRenderer::projectAllVertices( int count ) {
        const static FixP halfWidth{HALF_XRES};
        const static FixP halfHeight{HALF_YRES};
        const static FixP two{2};

        for ( auto& vertex : mVertices ) {

            if ( count-- == 0 ) {
                return;
            }

            const FixP oneOver = divide( halfHeight, divide(vertex.first.mZ, two) );

            vertex.second.mX = halfWidth + multiply(vertex.first.mX, oneOver);
            vertex.second.mY = halfHeight - multiply(vertex.first.mY, oneOver);
        }
    }

    void CRenderer::drawCubeAt(const Vec3& center, TexturePair texture) {

        if (center.mZ <= kMinZCull) {
            return;
        }

        const static FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one, -one, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one, -one, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  one, -one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  one, -one });
        mVertices[ 4 ].first = ( center + Vec3{ -one, -one,  one });
        mVertices[ 5 ].first = ( center + Vec3{  one, -one,  one });
        mVertices[ 6 ].first = ( center + Vec3{ -one,  one,  one });
        mVertices[ 7 ].first = ( center + Vec3{  one,  one,  one });

        projectAllVertices(8);

        const auto ulz0 = mVertices[0].second;
        const auto urz0 = mVertices[1].second;
        const auto llz0 = mVertices[2].second;
        const auto lrz0 = mVertices[3].second;
        const auto ulz1 = mVertices[4].second;
        const auto urz1 = mVertices[5].second;
        const auto llz1 = mVertices[6].second;
        const auto lrz1 = mVertices[7].second;

        if (static_cast<int>(center.mX) <= 0 ) {
            drawWall( urz0.mX, urz1.mX,
                      urz0.mY, lrz0.mY,
                      urz1.mY, lrz1.mY,
                      texture.second, one);
        }


        if (static_cast<int>(center.mY) >= 0 ) {
            drawFloor(ulz1.mY, urz0.mY,
                      ulz1.mX, urz1.mX,
                      ulz0.mX, urz0.mX,
                      texture.first);
        }

        if (static_cast<int>(center.mY) <= 0 ) {
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      texture.first);
        }

        if (static_cast<int>(center.mX) >= 0 ) {
            drawWall(ulz1.mX, ulz0.mX,
                     ulz1.mY, llz1.mY,
                     urz0.mY, lrz0.mY,
                     texture.second, one);
        }

        drawWall( ulz0.mX, urz0.mX,
                  ulz0.mY, llz0.mY,
                  urz0.mY, lrz0.mY,
                  texture.second, one );
    }


    void CRenderer::drawBillboardAt(const Vec3 &center, std::shared_ptr<odb::NativeTexture> texture ) {
        if (center.mZ <= kMinZCull) {
            return;
        }

        const static FixP one{ 1 };
        const static FixP two{ 2 };
        const auto textureScale = one / two;
        const auto scaledCenter = Vec3{ center.mX, multiply(center.mY, one), center.mZ };

        mVertices[ 0 ].first = ( scaledCenter + Vec3{ -one,  two, 0 });
        mVertices[ 1 ].first = ( scaledCenter + Vec3{  one,  two, 0 });
        mVertices[ 2 ].first = ( scaledCenter + Vec3{ -one, 0, 0 });
        mVertices[ 3 ].first = ( scaledCenter + Vec3{  one, 0, 0 });

        projectAllVertices(4);

        const auto ulz0 = mVertices[0].second;
        const auto urz0 = mVertices[1].second;
        const auto llz0 = mVertices[2].second;
        const auto lrz0 = mVertices[3].second;

        if (kShouldDrawTextures) {
                drawFrontWall( ulz0.mX, ulz0.mY,
                               lrz0.mX, lrz0.mY,
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

        if (center.mZ <= kMinZCull) {
            return;
        }

        const static FixP one{ 1 };
        const static FixP two{ 2 };

        const auto halfScale = scale;
        const auto textureScale = halfScale / two;
        const auto scaledCenter = Vec3{ center.mX, multiply(center.mY, one), center.mZ };

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

        projectAllVertices(8);

        const auto ulz0 = mVertices[0].second;
        const auto urz0 = mVertices[1].second;
        const auto llz0 = mVertices[2].second;
        const auto lrz0 = mVertices[3].second;
        const auto ulz1 = mVertices[4].second;
        const auto urz1 = mVertices[5].second;
        const auto llz1 = mVertices[6].second;
        const auto lrz1 = mVertices[7].second;

        if (kShouldDrawTextures) {

            if ( enableAlpha && mask[ 1 ] ) {
                drawFrontWall( ulz1.mX, ulz1.mY,
                               lrz1.mX, lrz1.mY,
                               texture.first, (textureScale *  two), enableAlpha );
            }

            if ( mask[0] && static_cast<int>(center.mX) < 0 ) {
                drawWall( urz0.mX, urz1.mX,
                          urz0.mY, lrz0.mY,
                          urz1.mY, lrz1.mY,
                          texture.second, (textureScale *  two));
            }

            if ( mask[2] && static_cast<int>(center.mX) > 0 ) {
                drawWall(ulz1.mX, ulz0.mX,
                         ulz1.mY, llz1.mY,
                         urz0.mY, lrz0.mY,
                         texture.second, (textureScale *  two));
            }

            if ( mask[ 1 ] ) {
                drawFrontWall( ulz0.mX, ulz0.mY,
                               lrz0.mX, lrz0.mY,
                               texture.first, (textureScale *  two), enableAlpha );
            }

            if ( mask[ 3 ] ) {
                drawMask( ulz0.mX, ulz0.mY,
                               lrz0.mX, lrz0.mY);
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

        if (center.mZ <= kMinZCull) {
            return;
        }

        const static FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one,  0, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one,  0, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  0,  one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  0,  one });

        projectAllVertices(4);

        const auto llz0 = mVertices[0].second;
        const auto lrz0 = mVertices[1].second;
        const auto llz1 = mVertices[2].second;
        const auto lrz1 = mVertices[3].second;

        if ( kShouldDrawTextures && static_cast<int>(center.mY) <= 0 ) {
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      texture.first);
        }



        if ( kShouldDrawOutline) {
            drawLine( llz0, lrz0 );
            drawLine( llz0, llz1 );
            drawLine( lrz0, lrz1 );
            drawLine( llz1, lrz1 );
        }
    }

    void CRenderer::drawCeilingAt(const Vec3& center, TexturePair texture) {

        if (center.mZ <= kMinZCull) {
            return;
        }

        const static FixP one{ 1 };

        mVertices[ 0 ].first = ( center + Vec3{ -one,  0, -one });
        mVertices[ 1 ].first = ( center + Vec3{  one,  0, -one });
        mVertices[ 2 ].first = ( center + Vec3{ -one,  0,  one });
        mVertices[ 3 ].first = ( center + Vec3{  one,  0,  one });

        projectAllVertices(4);

        const auto llz0 = mVertices[0].second;
        const auto lrz0 = mVertices[1].second;
        const auto llz1 = mVertices[2].second;
        const auto lrz1 = mVertices[3].second;

        if ( kShouldDrawTextures && static_cast<int>(center.mY) >= 0 ) {
            drawFloor(llz1.mY, lrz0.mY,
                      llz1.mX, lrz1.mX,
                      llz0.mX, lrz0.mX,
                      texture.first);
        }

        if (kShouldDrawOutline) {
            drawLine( llz0, lrz0 );
            drawLine( llz0, llz1 );
            drawLine( lrz0, lrz1 );
            drawLine( llz1, lrz1 );
        }

    }

    void CRenderer::drawLeftNear(const Vec3& center, const FixP &scale, std::shared_ptr<odb::NativeTexture> texture, bool mask[4]) {

        if (center.mZ <= kMinZCull) {
            return;
        }

        const static FixP one{ 1 };
        const static FixP two{ 2 };
        const auto halfScale = scale;
        const auto textureScale = halfScale;

        FixP depth{1};

        if (mCameraDirection == Knights::EDirection::kWest || mCameraDirection == Knights::EDirection::kEast ) {
            depth = FixP{-1};
        }

        mVertices[ 0 ].first = ( center + Vec3{ -one,  halfScale, -depth });
        mVertices[ 1 ].first = ( center + Vec3{  one,  halfScale, depth });
        mVertices[ 2 ].first = ( center + Vec3{ -one, -halfScale, -depth });
        mVertices[ 3 ].first = ( center + Vec3{  one, -halfScale, depth });

        projectAllVertices(4);

        const auto ulz0 = mVertices[0].second;
        const auto urz0 = mVertices[1].second;
        const auto llz0 = mVertices[2].second;
        const auto lrz0 = mVertices[3].second;

        if (kShouldDrawTextures) {
            drawWall( ulz0.mX, urz0.mX,
                      ulz0.mY, llz0.mY,
                      urz0.mY, lrz0.mY,
                      texture, textureScale );


            if (mask[3]) {

                mVertices[ 0 ].first = ( center + Vec3{ -one, -halfScale, -one });
                mVertices[ 1 ].first = ( center + Vec3{  one,  halfScale, -one });

                projectAllVertices(2);

                drawMask( mVertices[ 0 ].second.mX, mVertices[ 0 ].second.mY,
                          mVertices[ 1 ].second.mX, mVertices[ 1 ].second.mY);
            }
        }

        if (kShouldDrawOutline){
            drawLine( ulz0, urz0 );
            drawLine( llz0, ulz0 );
            drawLine( llz0, lrz0 );
            drawLine( urz0, lrz0 );
        }
    }


    void CRenderer::drawRightNear(const Vec3& center, const FixP &scale, std::shared_ptr<odb::NativeTexture> texture, bool mask[4]) {

        if (center.mZ <= kMinZCull) {
            return;
        }

        const static FixP one{ 1 };
        const static FixP two{ 2 };
        const auto halfScale = scale;
        const auto textureScale = halfScale ;

        FixP depth{1};

        if (mCameraDirection == Knights::EDirection::kWest || mCameraDirection == Knights::EDirection::kEast ) {
            depth = FixP{-1};
        }

        mVertices[ 0 ].first = ( center + Vec3{ -one,  halfScale, depth });
        mVertices[ 1 ].first = ( center + Vec3{  one,  halfScale, -depth });
        mVertices[ 2 ].first = ( center + Vec3{ -one, -halfScale, depth });
        mVertices[ 3 ].first = ( center + Vec3{  one, -halfScale, -depth });

        projectAllVertices(4);

        const auto ulz0 = mVertices[0].second;
        const auto urz0 = mVertices[1].second;
        const auto llz0 = mVertices[2].second;
        const auto lrz0 = mVertices[3].second;

        if (kShouldDrawTextures) {
            drawWall( ulz0.mX, urz0.mX,
                      ulz0.mY, llz0.mY,
                      urz0.mY, lrz0.mY,
                      texture, textureScale );

            if (mask[3]) {

                mVertices[ 0 ].first = ( center + Vec3{ -one, -halfScale, -one });
                mVertices[ 1 ].first = ( center + Vec3{  one,  halfScale, -one });

                projectAllVertices(2);

                drawMask( mVertices[ 0 ].second.mX, mVertices[ 0 ].second.mY,
                          mVertices[ 1 ].second.mX, mVertices[ 1 ].second.mY);
            }
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
        FixP u{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        //we can use this statically, since the textures are already loaded.
        //we don't need to fetch that data on every run.
        const uint8_t * data = texture->data();
        const int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        const FixP textureSize{ textureWidth };

        const FixP du = textureSize / (dX);
        auto ix = x;
        uint8_t * bufferData = getBufferData();

        for (; ix < limit; ++ix ) {
            if ( ix >= 0 && ix < XRES ) {

                const FixP diffY = (y1 - y0) / textureScaleY;

                if (diffY == 0) {
                    continue;
                }

                const FixP dv = textureSize / diffY;
                FixP v{0};
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

    uint8_t * CRenderer::getBufferData() {
        return &mBuffer[0];
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

        FixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);

        uint8_t* data = texture->data();
        const int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        const FixP textureSize{ textureWidth };

        const FixP dv = textureSize / (dY);

        const auto diffX = ( x1 - x0 );

        auto iX0 = static_cast<int16_t >(x0);
        auto iX1 = static_cast<int16_t >(x1);

        if ( iX0 == iX1 ) {
            //degenerate case
            return;
        }

        const FixP du = textureSize / diffX;

        uint8_t * bufferData = getBufferData();

        for (; iy < limit; ++iy ) {
            if (iy < YRES && iy >= 0) {
                FixP u{0};
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

        FixP v{0};

        //0xFF here acts as a dirty value, indicating there is no last value. But even if we had
        //textures this big, it would be only at the end of the run.
        uint8_t lastU = 0xFF;
        uint8_t lastV = 0xFF;

        auto iy = static_cast<int16_t >(y);

        uint8_t * bufferData = getBufferData();
        uint8_t * data = texture->data();
        const int8_t textureWidth = NATIVE_TEXTURE_SIZE;
        const FixP textureSize{ textureWidth };

        const FixP dv = textureSize / (dY);

        for (; iy < limit; ++iy ) {

            if ( iy < YRES && iy >= 0 ) {

                const auto diffX = (x1 - x0);

                if (diffX == 0) {
                    continue;
                }

                auto iX0 = static_cast<int16_t >(x0);
                auto iX1 = static_cast<int16_t >(x1);

                const FixP du = textureSize / diffX;
                FixP u{0};
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

    bool isOccluder(Knights::ElementView tile  ) {
        std::string occluderString = "1IYXR\\/SZ|%<>";

        for (const auto& candidate : occluderString ) {
            if (candidate == tile) {
                return true;
            }
        }
        return false;
    }


    void CRenderer::render(long ms) {

        if ( mNeedsToRedraw ) {
            const static FixP zero{0};
            const static FixP two{2};
            const static FixP eight{6};
            const static FixP one{1};

            mNeedsToRedraw = false;
#ifndef SDLSW
            auto t0 = uclock();
#endif
            if ( kShouldDrawSkybox ) {
                drawFrontWall(zero, zero, FixP{ XRES}, FixP{ YRES}, skybox, FixP{1});
            } else {
#ifdef SDLSW
                clear();
#endif
                auto bufferStart = getBufferData();
                std::fill( getBufferData(), getBufferData() + (320 * 128 ), 0 );
            }

            bool facesMask[4];

            EActorsSnapshotElement actorsSnapshotElement = EActorsSnapshotElement::kNothing;
            EItemsSnapshotElement itemsSnapshotElement = EItemsSnapshotElement::kNothing;

            CTile3DProperties tileProp;
            FixP heightDiff;
            FixP twiceHeight;
            FixP halfHeightDiff;
            Knights::ElementView lastElement = 0;
            Knights::ElementView element;
            Vec3 position;

            const auto cameraHeight = - multiply( two, mTileProperties[ mElementsMap[ mCameraPosition.y ][mCameraPosition.x ] ].mFloorHeight );
            for ( int distance = 79; distance >= 0; --distance ) {
                for ( const auto& visPos : distances[ distance ] ) {

                    int x = 0;
                    int z = 0;

                    facesMask[ 0 ] = true;
                    facesMask[ 1 ] = true;
                    facesMask[ 2 ] = true;
                    facesMask[ 3 ] = false;

                    switch (mCameraDirection) {
                        case Knights::EDirection::kNorth:
                            x = (Knights::kMapSize - 1) - visPos.x;
                            z = visPos.y;

                            element = mElementsMap[z][(Knights::kMapSize - 1) - x];

                            actorsSnapshotElement = mActors[z][(Knights::kMapSize - 1) - x ];
                            itemsSnapshotElement = mItems[z][(Knights::kMapSize - 1) - x ];

                            mCamera.mX = FixP{ 78 - ( 2 * mCameraPosition.x ) };
                            mCamera.mY = FixP{ cameraHeight -1};
                            mCamera.mZ = FixP{ ( 2 * mCameraPosition.y ) - 79};



                            position.mX = mCamera.mX + FixP{- 2 * x};
                            position.mY = mCamera.mY;
                            position.mZ = mCamera.mZ + FixP{ 80 - ( 2 * z)};

                            //remember, bounds - 1!
                            if ( lastElement != element ) {
                                facesMask[ 0 ] = !( x > 0 && mElementsMap[z][(Knights::kMapSize - 1) - (x - 1)] == element);
                                facesMask[ 2 ] = !( x < (Knights::kMapSize - 1) && mElementsMap[z][(Knights::kMapSize - 1) - (x + 1)] == element);
                                facesMask[ 1 ] = !( z < (Knights::kMapSize ) && mElementsMap[z + 1][(Knights::kMapSize - 1) - x] == element);
                            }

                            if ( z == mCameraPosition.y - 1 ) {

                                if (kShouldDrawBlackMasks && isOccluder(mElementsMap[z + 1][(Knights::kMapSize - 1) - x])) {
                                    facesMask[ 1 ] = false;
                                    facesMask[ 3 ] = true;
                                } else {
                                    facesMask[ 1 ] = true;
                                }
                            }

                            break;

                        case Knights::EDirection::kSouth:
                            x = visPos.x;
                            z = (Knights::kMapSize - 1) - visPos.y;

                            element = mElementsMap[(Knights::kMapSize - 1) - z][x];

                            actorsSnapshotElement = mActors[(Knights::kMapSize - 1) - z][x];
                            itemsSnapshotElement = mItems[(Knights::kMapSize - 1) - z][ x ];

                            mCamera.mX = FixP{ ( 2 * mCameraPosition.x ) };
                            mCamera.mY = FixP{cameraHeight -1};
                            mCamera.mZ = FixP{ ( 2 * ((Knights::kMapSize - 1) - mCameraPosition.y) ) - 79};

                            position.mX = mCamera.mX + FixP{- 2 * x};
                            position.mY = mCamera.mY;
                            position.mZ = mCamera.mZ + FixP{ 80 - ( 2 * z)};

                            //remember, bounds - 1!
                            if ( lastElement != element ) {
                                facesMask[0] = !(x > 0 && mElementsMap[(Knights::kMapSize - 1) - z][(x - 1)] == element);
                                facesMask[2] = !(x < (Knights::kMapSize - 1) && mElementsMap[(Knights::kMapSize - 1) - z][(x + 1)] == element);
                                facesMask[1] = !(z < (Knights::kMapSize ) && mElementsMap[(Knights::kMapSize - 1) - (z + 1)][x] == element);
                            }

                            if ( z == ((Knights::kMapSize - 1) - mCameraPosition.y) - 1 ) {

                                if (kShouldDrawBlackMasks && isOccluder(mElementsMap[(Knights::kMapSize - 1) - (z + 1)][x])) {
                                    facesMask[ 1 ] = false;
                                    facesMask[ 3 ] = true;
                                } else {
                                    facesMask[ 1 ] = true;
                                }
                            }


                            break;
                        case Knights::EDirection::kWest:
                            x = visPos.y;
                            z = (Knights::kMapSize - 1) - visPos.x;

                            element = mElementsMap[x][(Knights::kMapSize - 1) - z];

                            itemsSnapshotElement = mItems[x][(Knights::kMapSize - 1) - z ];
                            actorsSnapshotElement = mActors[x][(Knights::kMapSize - 1) - z ];

                            mCamera.mX = FixP{ ( 2 * mCameraPosition.y ) };
                            mCamera.mY = FixP{cameraHeight-1};
                            mCamera.mZ =FixP{ ( 2 * mCameraPosition.x ) - 1 };

                            position.mX = mCamera.mX + FixP{- 2 * x};
                            position.mY = mCamera.mY;
                            position.mZ = mCamera.mZ + FixP{ ( 2 * z) - 76};

                            //remember, bounds - 1!
                            if ( lastElement != element ) {
                                facesMask[0] = !(x > 0 && mElementsMap[x - 1][(Knights::kMapSize - 1) - z] == element);
                                facesMask[2] = !(x < (Knights::kMapSize - 1) && mElementsMap[x + 1][(Knights::kMapSize - 1) - z] == element);
                                facesMask[1] = !(z > 0 && mElementsMap[x][(Knights::kMapSize - 1) - (z - 1)] == element);
                            }

                            if ( z == ((Knights::kMapSize - 1) - mCameraPosition.x) + 1 ) {

                                if (kShouldDrawBlackMasks && isOccluder(mElementsMap[x][(Knights::kMapSize - 1) - (z - 1)])) {
                                    facesMask[ 1 ] = false;
                                    facesMask[ 3 ] = true;
                                } else {
                                    facesMask[ 1 ] = true;
                                }

                            }
                            break;

                        case Knights::EDirection::kEast:
                                x = visPos.y;
                                z = visPos.x;

                            element = mElementsMap[x][z];

                            actorsSnapshotElement = mActors[x][z ];
                            itemsSnapshotElement = mItems[x][z ];



                            mCamera.mX = FixP{ - ( 2 * mCameraPosition.y ) };
                            mCamera.mY = FixP{cameraHeight-1};
                            mCamera.mZ = FixP{ 79 - ( 2 * mCameraPosition.x ) };

                            position.mX = mCamera.mX + FixP{- 2 * ( - x)};
                            position.mY = mCamera.mY;
                            position.mZ = mCamera.mZ + FixP{ ( 2 * z) - 78};

                            //remember, bounds - 1!
                            if ( lastElement != element ) {
                                facesMask[2] = !(x > 0 && mElementsMap[x - 1][z] == element);
                                facesMask[0] = !(x < (Knights::kMapSize - 1) && mElementsMap[x + 1][z] == element);
                                facesMask[1] = !(z < (Knights::kMapSize - 1) && mElementsMap[x][(z - 1)] == element);
                            }

                            if ( z == (mCameraPosition.x) + 1 ) {

                                if (kShouldDrawBlackMasks && isOccluder(mElementsMap[x][(z - 1)])) {
                                    facesMask[ 1 ] = false;
                                    facesMask[ 3 ] = true;
                                } else {
                                    facesMask[ 1 ] = true;
                                }

                            }
                            break;

                    }

                    if ( lastElement != element ) {
                        tileProp = mTileProperties[element];
                        heightDiff = tileProp.mCeilingHeight - tileProp.mFloorHeight;
                        twiceHeight = multiply( heightDiff, two );
                        halfHeightDiff = heightDiff / two;
                    }

                    lastElement = element;

                    if ( tileProp.mFloorRepeatedTextureIndex > 0 && tileProp.mFloorRepetitions > 0) {

                        switch (tileProp.mGeometryType ) {
                            case kRightNearWall:
                                drawRightNear(
                                        position + Vec3{0, multiply(tileProp.mFloorHeight, two) - tileProp.mFloorRepetitions, 0},
                                        tileProp.mFloorRepetitions,
                                        mNativeTextures[tileProp.mFloorRepeatedTextureIndex].second,
                                        facesMask);

                                break;

                            case kLeftNearWall:
                                drawLeftNear(
                                        position + Vec3{0, multiply(tileProp.mFloorHeight, two) - tileProp.mFloorRepetitions, 0},
                                        tileProp.mFloorRepetitions,
                                        mNativeTextures[tileProp.mFloorRepeatedTextureIndex].second,
                                        facesMask);
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
                                        mNativeTextures[tileProp.mCeilingRepeatedTextureIndex].second,
                                        facesMask);
                                break;

                            case kLeftNearWall:
                                drawLeftNear(
                                        position + Vec3{0, multiply(tileProp.mCeilingHeight, two) + tileProp.mCeilingRepetitions, 0},
                                        tileProp.mCeilingRepetitions,
                                        mNativeTextures[tileProp.mCeilingRepeatedTextureIndex].second,
                                        facesMask);
                                break;

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
                                        mNativeTextures[ tileProp.mMainWallTextureIndex ].second,
                                        facesMask);
                                break;

                            case kLeftNearWall:
                                drawLeftNear(
                                        position + Vec3{ 0, multiply( tileProp.mFloorHeight, two) + heightDiff, 0},
                                        heightDiff,
                                        mNativeTextures[ tileProp.mMainWallTextureIndex ].second,
                                        facesMask);
                                break;

                            case kCube:
                            default:
                                drawColumnAt(
                                        position + Vec3{ 0, multiply( tileProp.mFloorHeight, two) + heightDiff, 0},
                                        heightDiff,

                                        mNativeTextures[ tileProp.mMainWallTextureIndex ],
                                        facesMask, tileProp.mNeedsAlphaTest );
                                break;
                        }
                    }



                    switch (actorsSnapshotElement) {
                        case EActorsSnapshotElement::kMonkAttacking0:
                            drawBillboardAt(
                                    position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                    monk );
                            break;
                        case EActorsSnapshotElement::kCocoonStanding0:
                            drawBillboardAt(
                                    position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                    cocoon );
                            break;
                        case EActorsSnapshotElement::kWarthogAttacking0:
                            drawBillboardAt(
                                    position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                    warthog );
                            break;
                        case EActorsSnapshotElement::kEvilSpiritAttacking0:
                            drawBillboardAt(
                                    position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                    specter );
                            break;
                        case EActorsSnapshotElement::kWeakenedDemonAttacking0:
                            drawBillboardAt(
                                    position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                    weakDemon );
                            break;
                        case EActorsSnapshotElement::kStrongDemonAttacking0:
                            drawBillboardAt(
                                    position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                    strongDemon );
                            break;
                        case EActorsSnapshotElement::kFallenAttacking0:
                            drawBillboardAt(
                                    position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                    foe );
                            break;
                        case EActorsSnapshotElement::kRope:
                            drawBillboardAt(
                                    position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                    rope );
                            break;
                        case EActorsSnapshotElement::kNothing:
                            break;
                    }



                    if ( itemsSnapshotElement != EItemsSnapshotElement ::kNothing ) {
                        switch (itemsSnapshotElement) {
                            case EItemsSnapshotElement::kCrossbow:
                                drawBillboardAt(
                                        position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                        bow
                                );
                                break;

                            case EItemsSnapshotElement::kShield:
                                drawBillboardAt(
                                        position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                        shield
                                );
                                break;

                            case EItemsSnapshotElement::kCross:
                                drawBillboardAt(
                                        position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                        token
                                );
                                break;

                            case EItemsSnapshotElement::kSword:
                                break;

                        }
                    }


                }
            }

            const static auto black = 0;

            fill( 320 - 64, 0, 64, 128, black );

            if (!mStaticPartsOfHudDrawn) {
                for ( int c = 0; c < 10; ++c ) {
                    drawSprite( c * 32, 128, mBackground );
                }

                mStaticPartsOfHudDrawn = true;
            }

            fill( 0, 160, 320, 40, black );

            switch (mCurrentItem) {
                case EItemsSnapshotElement::kCrossbow:
                    drawSprite(256 + 16, 8, bow );
                    break;

                case EItemsSnapshotElement::kShield:
                    drawSprite(256 + 16, 8, shield );
                    break;

                case EItemsSnapshotElement::kCross:
                    drawSprite(256 + 16, 8, token );
                    break;

                case EItemsSnapshotElement::kSword:
                    drawSprite(256 + 16, 8, sword );
                    break;
            }

            flip();

            char buffer[9];

            mItemName.resize(32, ' ');

            drawTextAt( 33, 7, mItemName.substr(0, 8).c_str() );
            drawTextAt( 33, 8, mItemName.substr(8, 8).c_str() );
            drawTextAt( 33, 9, mItemName.substr(16, 8).c_str() );

            if ( mCurrentItem == EItemsSnapshotElement::kShield || mCurrentItem == EItemsSnapshotElement::kCrossbow) {
                snprintf(buffer, 8, "%d/%d", mItemCapacity, mUsageCost);
                drawTextAt( 34, 10, buffer );
            }


            drawTextAt( 34, 12, "Faith:" );

            snprintf(buffer, 8, "%d", mHealth);
            drawTextAt( 34, 13, buffer );

            char directions[4] = {'N', 'E', 'S', 'W'};
            snprintf(buffer, 8, "Dir: %c", directions[static_cast<int>(mCameraDirection)]);
            drawTextAt( 34, 15, buffer );

            drawTextAt( 1, 21, mLogBuffer[0].c_str() );
            drawTextAt( 1, 22, mLogBuffer[1].c_str() );
            drawTextAt( 1, 23, mLogBuffer[2].c_str() );
            drawTextAt( 1, 24, mLogBuffer[3].c_str() );
            drawTextAt( 1, 25, mLogBuffer[4].c_str() );

#ifndef SDLSW
            auto t1 = uclock();
            mAccMs += (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
            mUsefulFrames++;
#endif
        }
    }

    void CRenderer::fill( int x, int y, int dx, int dy, uint8_t pixel ) {
        auto destination = getBufferData();

        for ( int py = 0; py < dy; ++py ) {
            auto destinationLineStart = destination + ( 320 * (y + py) ) + x;
            std::fill( destinationLineStart, destinationLineStart + dx, pixel );
        }
    }

    void CRenderer::drawSprite( int dx, int dy, std::shared_ptr<odb::NativeTexture> tile ) {
        auto destination = getBufferData();
        auto sourceLine = &((*tile)[0]);

        for ( int y = 0; y < 32; ++y ) {
            auto destinationLineStart = destination + ( 320 * (dy + y ) ) + dx;
            auto sourceLineStart = sourceLine + ( 32 * y );

            for ( int x = 0; x < 32; ++x ) {
                auto pixel = *sourceLineStart;

                if (pixel != CRenderer::mTransparency ) {
                    *destinationLineStart = pixel;
                }

                ++destinationLineStart;
                ++sourceLineStart;
            }

        }
    }

    void CRenderer::drawBitmap( int dx, int dy, std::shared_ptr<odb::NativeBitmap> tile ) {
        auto destination = getBufferData();
        auto sourceLine = tile->getPixelData();
        auto height = tile->getHeight();
        auto width = tile->getWidth();
        for ( int y = 0; y < height; ++y ) {
            auto destinationLineStart = destination + ( 320 * (dy + y ) ) + dx;
            auto sourceLineStart = sourceLine + ( width * y );
            for ( int x = 0; x < width; ++x ) {
                *destinationLineStart = getPaletteEntry(*sourceLineStart);
                ++sourceLineStart;
                ++destinationLineStart;
            }
        }
    }

    void CRenderer::appendToLog(const char* message) {
        mLogBuffer[0] = mLogBuffer[1];
        mLogBuffer[1] = mLogBuffer[2];
        mLogBuffer[2] = mLogBuffer[3];
        mLogBuffer[3] = mLogBuffer[4];
        mLogBuffer[4] = message;
    }


    void CRenderer::loadTextures(vector<vector<std::shared_ptr<odb::NativeBitmap>>> textureList, CTilePropertyMap &tile3DProperties) {
        mTextures.clear();
        mTileProperties.clear();
        mStaticPartsOfHudDrawn = false;
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
