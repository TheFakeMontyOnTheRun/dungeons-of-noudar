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

    const static bool kShouldDrawOutline = false;
    const static bool kShouldDrawTextures = true;
    const static auto kMinZCull = FixP{1};

    void CRenderer::projectAllVertices( uint8_t count ) {
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

    void CRenderer::drawLine(const Vec2& p0, const Vec2& p1) {
        drawLine(static_cast<int16_t >(p0.mX),
                 static_cast<int16_t >(p0.mY),
                 static_cast<int16_t >(p1.mX),
                 static_cast<int16_t >(p1.mY)
        );
    }
}
