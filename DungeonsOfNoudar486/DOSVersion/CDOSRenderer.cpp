#include <conio.h>
#include <iterator>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <utility>
#include <functional>
#include <memory>
#include <algorithm>
#include <map>
#include <unordered_map>
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
#include "LoadPNG.h"

namespace odb {

    long frame = 0;

    void CRenderer::putRaw(int16_t x, int16_t y, uint32_t pixel) {

        if (x < 0 || x >= 320 || y < 0 || y >= 128) {
            return;
        }

        mBuffer[(320 * y) + x] = pixel;
    }

    CRenderer::~CRenderer() {

        mNativeTextures.clear();

        textmode(C80);
        clrscr();
        printf("Thanks for playing!\nI would like to thank Ivan Odintsoff for the help,\nmy coworkers in the Lisbon office,\nmy family and most of all, my wife for keeping me human.\n\nDOS is back with a vengeance. This is only the first blow.\n\nSource code and licenses (DungeonsOfNoudar486):\nhttps://github.com/TheFakeMontyOnTheRun/dungeons-of-noudar\n\n" );
#ifdef PROFILEBUILD
        printf("Total time spent rendering: %d\nTotal time spent processing visibility: %d\nFrames rendered: %d\n", mAccMs, mProcVisTime, mUsefulFrames );
#endif
    }

    CRenderer::CRenderer(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {

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

        mFont = loadPNG("font.png", fileLoader );
    }

    void CRenderer::sleep(long ms) {
    }

    void CRenderer::handleSystemEvents() {
#ifdef PROFILEBUILD
        gotoxy(1,1);
        printf("%d", ++mFrame);
#endif
        const static FixP delta{2};

        int lastKey = 0;

        if (kbhit()) {
            auto getched = getch();
            switch(getched) {
                case 27:
                    mBufferedCommand = Knights::kQuitGameCommand;
                    break;

                case 'a':
                    mBufferedCommand = Knights::kPickItemCommand;
                    mCached = false;
                    break;
                case 's':
                    mBufferedCommand = Knights::kCycleRightInventoryCommand;
                    mCached = false;
                    break;

                case 'z':
                    mBufferedCommand = Knights::kStrafeLeftCommand;
                    mCached = false;
                    break;
                case 'x':
                    mBufferedCommand = Knights::kStrafeRightCommand;
                    mCached = false;
                    break;

                case ' ':
                    mBufferedCommand = Knights::kUseCurrentItemInInventoryCommand;
                    mCached = false;
                    break;

                case 224:
                case 0:
                    auto arrow = getch();
                    switch(arrow) {
                        case 75:
                            mBufferedCommand = Knights::kTurnPlayerLeftCommand;
                            mCached = false;
                            break;
                        case 72:
                            mBufferedCommand = Knights::kMovePlayerForwardCommand;
                            mCached = false;
                            break;
                        case 77:
                            mBufferedCommand = Knights::kTurnPlayerRightCommand;
                            mCached = false;
                            break;
                        case 80:
                            mBufferedCommand = Knights::kMovePlayerBackwardCommand;
                            mCached = false;
                            break;
                    }
                break;
            }
        }
    }

    void CRenderer::flip() {
        dosmemput(&mBuffer[0], 320 * 200, 0xa0000);
    }

    void CRenderer::clear() {
    }


    bool peekKeyboard() {
        return kbhit();
    }

    int readKeyboard() {
        return getch();
    }

}