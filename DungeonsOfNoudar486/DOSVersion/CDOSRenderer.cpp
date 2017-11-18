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

#include <conio.h>
#include <iterator>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>


namespace odb {

    long frame = 0;

    void CRenderer::putRaw(int16_t x, int16_t y, uint32_t pixel) {

        if (x < 0 || x >= 320 || y < 0 || y >= 128) {
            return;
        }

        mBuffer[(320 * y) + x] = pixel;
    }

    CRenderer::CRenderer() {

        __dpmi_regs reg;

        reg.x.ax = 0x13;
        __dpmi_int(0x10, &reg);

        outp(0x03c8, 0);

        for (int r = 0; r < 4; ++r) {
            for (int g = 0; g < 8; ++g) {
                for (int b = 0; b < 8; ++b) {
                    outp(0x03c9, (r * (21)));
                    outp(0x03c9, (g * (8)));
                    outp(0x03c9, (b * (8)));
                }
            }
        }
    }

    void CRenderer::sleep(long ms) {
    }

    void CRenderer::handleSystemEvents() {

        const static FixP delta{2};

        auto lastKey = bioskey(0x11);

        bdos(0xC, 0, 0);

        switch (lastKey) {
            case 18656:
                //up
                mSpeed.mY -= delta;
                mNeedsToRedraw = true;
                break;

            case 8051:
            case 20704:
                //down
                mSpeed.mY += delta;
                mNeedsToRedraw = true;
                break;

            case 19424: //right arrow
            case 4209: //q
                //left
                mSpeed.mX += delta;
                mNeedsToRedraw = true;
                break;

            case 4709: //e
            case 19936: //right arrow
                //right
                mSpeed.mX -= delta;
                mNeedsToRedraw = true;
                break;

            case 3849:
            case 14624:
            case 11785: //c
            case 5236: //t
                //space
                exit(0);
                break;
            case 561:
                clearScr = true;
                mNeedsToRedraw = true;
                break;
            case 818:
                clearScr = false;
                mNeedsToRedraw = true;
                break;

            case 7777:
                mSpeed.mZ -= delta;
                mNeedsToRedraw = true;
                break;

            case 11386:
                mSpeed.mZ += delta;
                mNeedsToRedraw = true;
                break;

            case 0:
                break;
            default:
                printf("WTF is %d", lastKey);
                exit(0);

        }

    }

    void CRenderer::flip() {
        dosmemput(&mBuffer[0], 320 * 128, 0xa0000);
    }

    void CRenderer::clear() {
        std::fill( std::begin(mBuffer), std::end(mBuffer), 0);
    }
}