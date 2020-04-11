#include <assert.h>
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
#include <vector>
#include <array>

using sg14::fixed_point;
using std::vector;
using std::array;
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

    const static bool kShouldDrawSkybox = false;
    const static bool kShouldDrawBlackMasks = true;
    const static long kSplatFrameTime = 200;
    const static long kHighlightTime = 300;

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
    std::shared_ptr<odb::NativeTexture> splat[3];
    std::shared_ptr<NativeTexture > skybox;

    VisMap visMap;
    IntMap intMap;
    DistanceDistribution distances;

    vector<std::string> splitLists(const std::string& data) {
        vector<std::string> toReturn;
        auto buffer = std::string(data);
        buffer.push_back('\n');
        auto lastPoint = 0;
        auto since = 0;
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

    odb::CTilePropertyMap loadTileProperties( uint8_t levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {
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
        const auto ratio = width / NATIVE_TEXTURE_SIZE;

        for (auto y = 0; y < NATIVE_TEXTURE_SIZE; ++y) {
            for (auto x = 0; x < NATIVE_TEXTURE_SIZE; ++x) {
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
        const auto ratio = width / NATIVE_TEXTURE_SIZE;

        for ( auto y = 0; y < NATIVE_TEXTURE_SIZE; ++y ) {
            for ( auto x = 0; x < NATIVE_TEXTURE_SIZE; ++x ) {
                const uint32_t pixel = bitmap->getPixelData()[ ( width * (y * ratio ) ) + (x * ratio ) ];
                const auto converted = CRenderer::getPaletteEntry( pixel );
                nativeTexture->data()[ ( NATIVE_TEXTURE_SIZE * y ) + x ] = converted;
                rotatedTexture->data()[ ( NATIVE_TEXTURE_SIZE * x ) + y ] = converted;
            }
        }

        return std::make_pair(nativeTexture, rotatedTexture );
    }

    vector<std::shared_ptr<odb::NativeBitmap>>
    loadTexturesForLevel(uint8_t levelNumber, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {
        char tilesFilename[64];
        snprintf(tilesFilename, 64, "tiles%d.lst", levelNumber);
        const auto data = fileLoader->loadFileFromPath( tilesFilename );
        const auto list = splitLists(data);

        vector<std::shared_ptr<odb::NativeBitmap>> tilesToLoad;

        for ( const auto& frameList : list ) {
            tilesToLoad.push_back( loadPNG(frameList, fileLoader) );
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
        splat[0] = makeTexture("splat2.png", fileLoader);
        splat[1] = makeTexture("splat1.png", fileLoader);
        splat[2] = makeTexture("splat0.png", fileLoader);

        if ( kShouldDrawSkybox) {
            skybox = makeTexture("clouds.png", fileLoader );
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
                mSplats[z][x] = -1;

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
#ifdef PROFILEBUILD
        auto t0 = uclock();
#endif

        VisibilityStrategy::castVisibility(visMap, intMap, mCameraPosition, mCameraDirection, true, distances);
#ifdef PROFILEBUILD
        auto t1 = uclock();
        mProcVisTime += (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
#endif
    }

    Knights::CommandType CRenderer::peekInput() {
        return mBufferedCommand;
    }

    Knights::CommandType CRenderer::getInput() {
        const auto toReturn = mBufferedCommand;
        mBufferedCommand = Knights::kNullCommand;
        return toReturn;
    }

    bool isOccluder(Knights::ElementView tile  ) {
        std::string occluderString = "1IYXPR\\/SZ|%<>";

        for (const auto& candidate : occluderString ) {
            if (candidate == tile) {
                return true;
            }
        }
        return false;
    }

    uint8_t* CRenderer::getBufferData() {
        return mBuffer.data();
    }

    void CRenderer::render(long ms) {

        if ( mNeedsToRedraw ) {
            mHighlightTime -= ms;
            mSplatFrameTime -= ms;

            const static FixP zero{0};
            const static FixP two{2};
            const static FixP eight{6};
            const static FixP one{1};

            mNeedsToRedraw = false;
#ifdef PROFILEBUILD
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
            bool splatDrawn = false;
            auto shouldSplat = -1;
            const auto cameraHeight = - multiply( two, mTileProperties[ mElementsMap[ mCameraPosition.y ][mCameraPosition.x ] ].mFloorHeight );

            switch (mCameraDirection) {
                case Knights::EDirection::kNorth:
                    mCamera.mX = FixP{ 78 - ( 2 * mCameraPosition.x ) };
                    mCamera.mY = FixP{ cameraHeight -1};
                    mCamera.mZ = FixP{ ( 2 * mCameraPosition.y ) - 79};
                    break;

                case Knights::EDirection::kSouth:
                    mCamera.mX = FixP{ ( 2 * mCameraPosition.x ) };
                    mCamera.mY = FixP{cameraHeight -1};
                    mCamera.mZ = FixP{ ( 2 * ((Knights::kMapSize - 1) - mCameraPosition.y) ) - 79};
                    break;

                case Knights::EDirection::kWest:
                    mCamera.mX = FixP{ ( 2 * mCameraPosition.y ) };
                    mCamera.mY = FixP{cameraHeight-1};
                    mCamera.mZ =FixP{ ( 2 * mCameraPosition.x ) - 1 };
                    break;

                case Knights::EDirection::kEast:
                    mCamera.mX = FixP{ - ( 2 * mCameraPosition.y ) };
                    mCamera.mY = FixP{cameraHeight-1};
                    mCamera.mZ = FixP{ 79 - ( 2 * mCameraPosition.x ) };
                    break;

            }

            for ( auto distance = 79; distance >= 0; --distance ) {
                for ( const auto& visPos : distances[ distance ] ) {

                    auto x = 0;
                    auto z = 0;

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
                            shouldSplat = mSplats[z][(Knights::kMapSize - 1) - x ];

                            if (mSplatFrameTime < 0) {
                                mSplats[z][(Knights::kMapSize - 1) - x ]--;
                            }

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
                            shouldSplat = mSplats[(Knights::kMapSize - 1) - z][ x ];

                            if (mSplatFrameTime < 0) {
                                mSplats[(Knights::kMapSize - 1) - z][ x ]--;
                            }

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
                            shouldSplat = mSplats[x][(Knights::kMapSize - 1) - z ];

                            if (mSplatFrameTime < 0) {
                                mSplats[x][(Knights::kMapSize - 1) - z ]--;
                            }

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
                            shouldSplat = mSplats[x][z ];

                            if (mSplatFrameTime < 0) {
                                mSplats[x][z ]--;
                            }

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

                    if (shouldSplat >= 0 ) {
                        drawBillboardAt(
                                position + Vec3{ 0, multiply( tileProp.mFloorHeight, two), 0},
                                splat[ shouldSplat ] );
                        mNeedsToRedraw = true;
                        splatDrawn = true;

                        if ( shouldSplat > 0 && mSplatFrameTime < 0 ) {
                            mSplatFrameTime = kSplatFrameTime;
                        }
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
            const static auto red = getPaletteEntry( 0xFF0000FF );
            const static auto green = getPaletteEntry( 0xFF00FF00 );

            auto backgroundColour = black;

            if ( mDamageHighlight > 0 && !splatDrawn ) {
                if (mHighlightTime <= 0 ) {
                    mDamageHighlight = 0;
                }

                backgroundColour = red;
                mNeedsToRedraw = true;
            }

            if ( mHealHighlight > 0 && !splatDrawn ) {
                if (mHighlightTime <= 0 ) {
                    mHealHighlight = 0;
                }
                backgroundColour = green;
                mNeedsToRedraw = true;
            }

            for ( auto c = 0; c < 2; ++c ) {
                drawSprite( 320 - 32, 16 + 64 + ( c * 32), mBackground );
                drawSprite( 320 - 64, 16 + 64 + ( c * 32), mBackground );
            }

            fill( 320 - 64, 0, 64, 64 + 16, backgroundColour );
            fill( 320 - 64 + 8, 64 + 16, 32 + 16, 64 - 16, backgroundColour );

            if (!mStaticPartsOfHudDrawn) {
                for ( auto c = 0; c < 10; ++c ) {
                    drawSprite( c * 32, 128, mBackground );
                }
                mStaticPartsOfHudDrawn = true;
            }



//            fill( 0, 160, 320, 40, black );

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

            fill(0, 160, 320, 40, black );
            drawTextAt( 1, 21, &mLogBuffer[0][0], mLineColour[0] );
            drawTextAt( 1, 22, &mLogBuffer[1][0], mLineColour[1] );
            drawTextAt( 1, 23, &mLogBuffer[2][0], mLineColour[2] );
            drawTextAt( 1, 24, &mLogBuffer[3][0], mLineColour[3] );
            drawTextAt( 1, 25, &mLogBuffer[4][0], mLineColour[4] );

            flip();


#ifdef PROFILEBUILD
            auto t1 = uclock();
            mAccMs += (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
            mUsefulFrames++;
#endif
        }
    }

    void CRenderer::startHealHighlight() {
        mHealHighlight = 1;
        mHighlightTime = kHighlightTime;
    }

    void CRenderer::startDamageHighlight() {
        mDamageHighlight = 1;
        mHighlightTime = kHighlightTime;
    }

    void CRenderer::addSplatAt( const Knights::Vec2i& position ) {
        mSplats[position.y][position.x] = 0;
        mSplatFrameTime = kSplatFrameTime;
    }

    void CRenderer::addDeathAt( const Knights::Vec2i& position ) {
        mSplats[position.y][position.x] = 2;
        mSplatFrameTime = kSplatFrameTime;
    }

    void CRenderer::appendToLog(const char* message, uint8_t colour = 15) {

        if ( message == nullptr || strlen(message) == 0 ) { //|| !strcmp( message, &mLogBuffer[4][0] ) ) {
            return;
        }

        snprintf(&mLogBuffer[0][0], 39, "%s", &mLogBuffer[1][0] );
        snprintf(&mLogBuffer[1][0], 39, "%s", &mLogBuffer[2][0] );
        snprintf(&mLogBuffer[2][0], 39, "%s", &mLogBuffer[3][0] );
        snprintf(&mLogBuffer[3][0], 39, "%s", &mLogBuffer[4][0] );
        snprintf(&mLogBuffer[4][0], 39, "%s", message );
        mLineColour[0] = mLineColour[1];
        mLineColour[1] = mLineColour[2];
        mLineColour[2] = mLineColour[3];
        mLineColour[3] = mLineColour[4];
        mLineColour[4] = colour;
    }


    void CRenderer::loadTextures(vector<std::shared_ptr<odb::NativeBitmap>> textureList, CTilePropertyMap &tile3DProperties) {
        mNativeTextures.clear();

        for ( const auto& bitmap : textureList ) {
            mNativeTextures.push_back(makeTexturePair(bitmap));
        }

        snprintf(&mLogBuffer[0][0], 39, "" );
        snprintf(&mLogBuffer[1][0], 39, "" );
        snprintf(&mLogBuffer[2][0], 39, "" );
        snprintf(&mLogBuffer[3][0], 39, "" );
        snprintf(&mLogBuffer[4][0], 39, "" );


        mTileProperties.clear();
        mStaticPartsOfHudDrawn = false;
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
        textureRegistry["begin"] = ETextures::Arch;
        textureRegistry["exit"] = ETextures::Arch;
        textureRegistry["bricksblood"] = ETextures::BricksBlood;
        textureRegistry["brickscandles"] = ETextures::BricksCandles;
        textureRegistry["ceiling"] = ETextures::Ceiling;
        textureRegistry["ceilingdoor"] = ETextures::CeilingDoor;
        textureRegistry["ceilingbegin"] = ETextures::CeilingBegin;
        textureRegistry["ceilingend"] = ETextures::CeilingEnd;
        textureRegistry["ceilingbars"] = ETextures::CeilingBars;
        textureRegistry["rope"] = ETextures::Rope;
        textureRegistry["slot"] = ETextures::Slot;
        textureRegistry["magicseal"] = ETextures::MagicSeal;
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
