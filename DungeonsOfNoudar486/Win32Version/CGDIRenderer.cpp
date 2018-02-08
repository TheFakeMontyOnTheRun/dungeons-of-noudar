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

#include <windows.h>

MSG Msg;
HWND myHWnd = nullptr;
bool needsRedraw = true;
uint32_t bitmap[320 * 200];

long timeEllapsed = 0;

long uclock() {
    timeEllapsed += 10;
    return timeEllapsed;
}

namespace odb {

    int readKeyboard(std::shared_ptr<CRenderer> renderer) {
        needsRedraw = true;
        while (renderer->mBufferedCommand != 13) {
            needsRedraw = true;
            renderer->handleSystemEvents();
        }
        renderer->mBufferedCommand = '.';

        return 13;
    }

    bool peekKeyboard(std::shared_ptr<CRenderer> renderer) {
        needsRedraw = true;
	renderer->handleSystemEvents();
        return renderer->mBufferedCommand == 13;
    }


    CRenderer::CRenderer(std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {

        for (int r = 0; r < 256; r += 16) {
            for (int g = 0; g < 256; g += 8) {
                for (int b = 0; b < 256; b += 8) {
                    auto pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                    auto paletteEntry = getPaletteEntry(pixel);
                    mPalette[paletteEntry] = pixel;
                }
            }
        }

        mFont = loadPNG("font.png", fileLoader);
    }


    LRESULT CALLBACK
    WindProcedure(HWND
                  hWnd,
                  UINT Msg, WPARAM
                  wParam,
                  LPARAM lParam
    ) {

        if (needsRedraw) {
            InvalidateRect(hWnd,
                           nullptr, false);
            needsRedraw = false;
        }

	HBITMAP gameBitmap = CreateBitmap(320, 200, 1, 32, &bitmap[0]);

        if (renderer != nullptr) {
		auto bufferPtr = renderer->getBufferData();

                for (int c = 0;c < 200; ++c) {
                	for (int d = 0;d < 320; ++d) {
                        	auto texel = renderer->mPalette[bufferPtr[(320 * c) + d]];
                                uint32_t fragment = 0;
                                fragment += (((texel & 0xFF) - 0x38) << 16);
                                fragment += ((((texel & 0xFF00) >> 8) - 0x18) << 8);
                                fragment += ((texel & 0xFF0000) >> 16) - 0x10;

                                bitmap[(320 * c) + d] = fragment;
			}
		}
	}

        HDC hDC, MemDCGame;
        PAINTSTRUCT Ps;

        switch (Msg) {

            case
                WM_CHAR:
                switch (wParam) {

                    case 'a':
                        renderer->
                                mBufferedCommand = Knights::kPickItemCommand;
                        renderer->
                                mCached = false;
                        break;

                    case 's':
                        renderer->
                                mBufferedCommand = Knights::kCycleRightInventoryCommand;
                        renderer->
                                mCached = false;
                        break;

                    case 'z':
                        renderer->
                                mBufferedCommand = Knights::kStrafeLeftCommand;
                        renderer->
                                mCached = false;
                        break;

                    case 'x':
                        renderer->
                                mBufferedCommand = Knights::kStrafeRightCommand;
                        renderer->
                                mCached = false;
                        break;


                    case '1':
                        renderer->
                                mBufferedCommand = '1';
                        renderer->
                                mCached = false;
                        break;

                    case '2':
                        renderer->
                                mBufferedCommand = '2';
                        renderer->
                                mCached = false;
                        break;

                    case '3':
                        renderer->
                                mBufferedCommand = '3';
                        renderer->
                                mCached = false;
                        break;
                }
                needsRedraw = true;
                break;
            case
                WM_KEYDOWN:

                switch (wParam) {

                    case
                        VK_ESCAPE:
                        renderer
                                ->
                                        mBufferedCommand = Knights::kQuitGameCommand;
                        break;
                    case
                        VK_SPACE:
                        renderer
                                ->
                                        mBufferedCommand = Knights::kUseCurrentItemInInventoryCommand;
                        renderer->
                                mCached = false;
                        break;


                    case
                        VK_LEFT:
                        renderer
                                ->
                                        mBufferedCommand = Knights::kTurnPlayerLeftCommand;
                        renderer->
                                mCached = false;
                        break;

                    case
                        VK_RIGHT:
                        renderer
                                ->
                                        mBufferedCommand = Knights::kTurnPlayerRightCommand;
                        renderer->
                                mCached = false;
                        break;

                    case
                        VK_UP:
                        renderer
                                ->
                                        mBufferedCommand = Knights::kMovePlayerForwardCommand;
                        renderer->
                                mCached = false;
                        break;

                    case
                        VK_RETURN:
                        renderer
                                ->
                                        mBufferedCommand = 13;
                        break;


                    case
                        VK_DOWN:
                        renderer
                                ->
                                        mBufferedCommand = Knights::kMovePlayerBackwardCommand;
                        renderer->
                                mCached = false;

                        break;
                }
                needsRedraw = true;
                break;

            case
                WM_DESTROY:
                PostQuitMessage(WM_QUIT);
                break;
            case
                WM_PAINT:




                hDC = BeginPaint(hWnd, &Ps);
                MemDCGame = CreateCompatibleDC(hDC);
                SelectObject(MemDCGame, gameBitmap
                );
                BitBlt(hDC,
                       0, 0, 320, 200, MemDCGame, 0, 0, SRCCOPY);
                DeleteDC(MemDCGame);
                EndPaint(hWnd, &Ps
                );

                break;
            default:
                return
                        DefWindowProc(hWnd, Msg, wParam, lParam);
        }

	DeleteObject(gameBitmap);
        return 0;
    }

    void CRenderer::sleep(long ms) {
    }

    void CRenderer::handleSystemEvents() {
        if (GetMessage(&Msg, NULL, 0, 0)) {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    void CRenderer::putRaw(int16_t x, int16_t y, uint32_t pixel) {


        if (x < 0 || x >= 256 || y < 0 || y >= 128) {
            return;
        }

        mBuffer[(320 * y) + x] = pixel;
    }

    CRenderer::~CRenderer() {
        mNativeTextures.clear();
    }

    void CRenderer::flip() {
        needsRedraw = true;
    }

    void CRenderer::clear() {
    }

}
