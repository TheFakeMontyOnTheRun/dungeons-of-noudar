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
#include <vector>
#include <array>

using std::vector;
using std::array;
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

long uclock() {
    return GetTickCount();
}

namespace odb {

    static HBITMAP Create8bppBitmap(HDC hdc, int width, int height, LPVOID pBits = NULL)
    {
        BITMAPINFO *bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
        BITMAPINFOHEADER &bih(bmi->bmiHeader);
        bih.biSize = sizeof (BITMAPINFOHEADER);
        bih.biWidth         = width;
        bih.biHeight        = -height;
        bih.biPlanes        = 1;
        bih.biBitCount      = 8;
        bih.biCompression   = BI_RGB;
        bih.biSizeImage     = 0;
        bih.biXPelsPerMeter = 14173;
        bih.biYPelsPerMeter = 14173;
        bih.biClrUsed       = 0;
        bih.biClrImportant  = 0;
        for (int I = 0; I <= 255; I++)
        {
            bmi->bmiColors[I].rgbBlue = bmi->bmiColors[I].rgbGreen = bmi->bmiColors[I].rgbRed = (BYTE)I;
            bmi->bmiColors[I].rgbReserved = 0;
        }

        void *Pixels = NULL;
        HBITMAP hbmp = CreateDIBSection(hdc, bmi, DIB_RGB_COLORS, &Pixels, NULL, 0);

        if(pBits != NULL)
        {
            //fill the bitmap
            BYTE* pbBits = (BYTE*)pBits;
            BYTE *Pix = (BYTE *)Pixels;
            memcpy(Pix, pbBits, width * height);
        }

        free(bmi);

        return hbmp;
    }

    static HBITMAP CreateBitmapFromPixels( HDC hDC, UINT uWidth, UINT uHeight, UINT uBitsPerPixel, LPVOID pBits)
    {
        if(uBitsPerPixel < 8) // NOT IMPLEMENTED YET
            return NULL;

        if(uBitsPerPixel == 8)
            return Create8bppBitmap(hDC, uWidth, uHeight, pBits);

        HBITMAP hBitmap = 0;
        if ( !uWidth || !uHeight || !uBitsPerPixel )
            return hBitmap;
        LONG lBmpSize = uWidth * uHeight * (uBitsPerPixel/8) ;
        BITMAPINFO bmpInfo = { 0 };
        bmpInfo.bmiHeader.biBitCount = uBitsPerPixel;
        bmpInfo.bmiHeader.biHeight = uHeight;
        bmpInfo.bmiHeader.biWidth = uWidth;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        // Pointer to access the pixels of bitmap
        UINT * pPixels = 0;
        hBitmap = CreateDIBSection( hDC, (BITMAPINFO *)&
                                         bmpInfo, DIB_RGB_COLORS, (void **)&
                pPixels , NULL, 0);

        if ( !hBitmap )
            return hBitmap; // return if invalid bitmaps

        //SetBitmapBits( hBitmap, lBmpSize, pBits);
        // Directly Write
        memcpy(pPixels, pBits, lBmpSize );

        return hBitmap;
    }

    int readKeyboard(std::shared_ptr<CRenderer> renderer) {
        needsRedraw = true;
        while (renderer->mBufferedCommand != 13) {
            needsRedraw = true;
            renderer->handleSystemEvents();
        }
        renderer->mBufferedCommand = Knights::kNullCommand;

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
                WM_PAINT: {
                    HBITMAP hBitmap = CreateBitmapFromPixels(hDC,320, 200, 8, renderer->getBufferData());
                    PAINTSTRUCT     ps;
                    HDC             hdc;
                    BITMAP          bitmap;
                    HDC             hdcMem;
                    HGDIOBJ         oldBitmap;

                    hdc = BeginPaint(hWnd, &ps);

                    hdcMem = CreateCompatibleDC(hdc);
                    oldBitmap = SelectObject(hdcMem, hBitmap);

                    GetObject(hBitmap, sizeof(bitmap), &bitmap);
                    BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

                    SelectObject(hdcMem, oldBitmap);
                    DeleteDC(hdcMem);

                    EndPaint(hWnd, &ps);
                }
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
