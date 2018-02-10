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
bool needsRedraw = true;
COLORREF paletteRef[256];
bool havePalette = false;
COLORREF transparencyRef;
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
                renderer
                ->
                mBufferedCommand = Knights::kQuitGameCommand;

                break;
            case
                WM_PAINT:

                hDC = BeginPaint(hWnd, &Ps);


                if (renderer != nullptr) {
                    uint8_t* bufferPtr = renderer->getBufferData();
                    auto palettePtr = &renderer->mPalette[0];

                    if ( !havePalette ) {
                        havePalette = true;
                        auto pixel = CRenderer::mTransparency;
                        auto r = (((pixel & 0x000000FF)      )) - 0x38;
                        auto g = (((pixel & 0x0000FF00) >>  8)) - 0x18;
                        auto b = (((pixel & 0x00FF0000) >> 16)) - 0x10;
                        transparencyRef = RGB(r, g, b);

                        int c = 0;
                        for ( const auto& pixel : renderer->mPalette ) {
                            auto r = (((pixel & 0x000000FF)      )) - 0x38;
                            auto g = (((pixel & 0x0000FF00) >>  8)) - 0x18;
                            auto b = (((pixel & 0x00FF0000) >> 16)) - 0x10;

                            paletteRef[c++] = transparencyRef;
                        }
                    }

                    RECT rect;
                    HBRUSH brush;
                    for (int c = 0;c < 200; ++c) {
                        uint8_t* line = &bufferPtr[ 320 * c ];
                        for (int d = 0;d < 320; ++d) {
                            uint8_t index = *line;
                            COLORREF ref = paletteRef[index];

                            if ( ref == transparencyRef ) {
                                auto pixel = palettePtr[index];
                                auto r = (((pixel & 0x000000FF)      )) - 0x38;
                                auto g = (((pixel & 0x0000FF00) >>  8)) - 0x18;
                                auto b = (((pixel & 0x00FF0000) >> 16)) - 0x10;
                                ref = RGB(r, g, b);
                                paletteRef[index] = ref;
                            }

                            SetPixel( hDC, d, c, ref);
                            ++line;
                        }
                    }
                    brush = CreateSolidBrush(RGB(0, 0, 0 ));

                    rect.left = 0;
                    rect.top = 200;
                    rect.right = 320;
                    rect.bottom = 240;

                    FillRect(Ps.hdc, &rect, brush);

                    DeleteObject(brush);
                }

                EndPaint(hWnd, &Ps);

                break;
            default:
                return
                        DefWindowProc(hWnd, Msg, wParam, lParam);
        }

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
