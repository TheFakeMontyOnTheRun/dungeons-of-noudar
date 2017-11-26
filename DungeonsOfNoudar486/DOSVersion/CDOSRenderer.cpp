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
                    outp(0x03c9, (r * (16)));
                    outp(0x03c9, (g * (8)));
                    outp(0x03c9, (b * (8)));
                }
            }
        }
    }

    void CRenderer::sleep(long ms) {
    }

    void CRenderer::handleSystemEvents() {
        gotoxy(1,1);
        printf("%d", ++mFrame);

        const static FixP delta{2};

        auto lastKey = bioskey(0x11);

        bdos(0xC, 0, 0);

        switch (lastKey) {
            case 18656: //up
                mBufferedCommand = Knights::kMovePlayerForwardCommand;
                mCached = false;
                break;

            case 20704: //down
                mBufferedCommand = Knights::kMovePlayerBackwardCommand;
                mCached = false;
                break;

            case 19424: //left
                mBufferedCommand = Knights::kTurnPlayerLeftCommand;
                mCached = false;
                break;

            case 4209: //q
                mBufferedCommand = Knights::kCycleLeftInventoryCommand;
                mCached = false;
                break;

            case 4471: //w
                mBufferedCommand = Knights::kPickItemCommand;
                mCached = false;
                break;

            case 4709: //e
                mBufferedCommand = Knights::kCycleRightInventoryCommand;
                mCached = false;
                break;

            case 8051: //s
                mBufferedCommand = Knights::kDropItemCommand;
                mCached = false;
                break;

            case 19936: //right arrow
                mBufferedCommand = Knights::kTurnPlayerRightCommand;
                mCached = false;
                break;

            case 14624: //space
                mBufferedCommand = Knights::kUseCurrentItemInInventoryCommand;
                mCached = false;
                break;

            case 283: //esc
                exit(0);
                break;

            case 11386: //z
                mBufferedCommand = Knights::kStrafeLeftCommand;
                mCached = false;
                break;
            case 11640: //x
                mBufferedCommand = Knights::kStrafeRightCommand;
                mCached = false;
                break;
            case 0:
                break;
            default:
                printf("WTF is %d", lastKey);
                exit(0);
        }
    }

    void CRenderer::drawTextAt( int x, int y, const char* text ) {
        gotoxy(x, y );
        printf("%s", text);
    }

    void CRenderer::flip() {
        dosmemput(&mBuffer[0], 320 * 200, 0xa0000);
    }

    void CRenderer::clear() {
    }
}