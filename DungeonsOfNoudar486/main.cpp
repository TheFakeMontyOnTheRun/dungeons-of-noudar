#if defined(WINVER)
#include <windows.h>
#endif

#ifdef __DJGPP__
#else
const long UCLOCKS_PER_SEC = 1000;

long uclock();

#endif

#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <memory>
#include <utility>
#include <map>
#include <algorithm>
#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

enum class ESoundDriver {
    kNone, kPcSpeaker, kOpl2Lpt, kAdlib, kTandy, kCovox
};

ESoundDriver soundDriver = ESoundDriver::kNone;
void initOPL2(int port);
void playTune(const std::string &);

///game interface
void init();
void loopTick(long ms);
void shutdown();
bool  isPlaying();
///
#ifdef __EMSCRIPTEN__
void emscriptenLoopTick() {
    loopTick(50);
}
#endif

#ifdef __APPLE__
extern "C" int SDL_main(int argc, char **argv) {
#else

#if defined(WINVER)

namespace odb {
    LRESULT CALLBACK WindProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{

        HINSTANCE hInst;

        WNDCLASSEX WndCls;
        static char szAppName[] = "DungeonsOfNoudar95";
        MSG Msg;

        hInst = hInstance;
        WndCls.cbSize = sizeof(WndCls);
        WndCls.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        WndCls.lpfnWndProc = odb::WindProcedure;
        WndCls.cbClsExtra = 0;
        WndCls.cbWndExtra = 0;
        WndCls.hInstance = hInst;
        WndCls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        WndCls.hCursor = LoadCursor(NULL, IDC_ARROW);
        WndCls.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
        WndCls.lpszMenuName = NULL;
        WndCls.lpszClassName = szAppName;
        WndCls.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
        RegisterClassEx(&WndCls);

        CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                       szAppName,
                       "Dungeons of Noudar 95",
                       WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       320,
                       240,
                       NULL,
                       NULL,
                       hInstance,
                       NULL);

#else

int main(int argc, char **argv) {
    puts("Dungeons of Noudar 3D startup. Gonna load some stuff...");

    if (argc >= 2) {
        if (!std::strcmp(argv[1], "opl2lpt")) {
            soundDriver = ESoundDriver::kOpl2Lpt;
            initOPL2(-1);
            playTune("t200i101o1a");
        } else if (!std::strcmp(argv[1], "adlib")) {
            soundDriver = ESoundDriver::kAdlib;
            initOPL2(0x0388);
            playTune("t200i101o1a");
        } else if (!std::strcmp(argv[1], "pcspeaker")) {
            soundDriver = ESoundDriver::kPcSpeaker;
        }
    }

#endif

#endif

    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscriptenLoopTick, 20, 1);
#else
    clock_t diff = 0;

    while (isPlaying()) {
        clock_t t0 = 0;
        clock_t t1 = 0;

        t0 = uclock();

        loopTick(diff);

        t1 = uclock();

        diff = (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;

        if (diff == 0) {
            diff = 1;
        }
    }
#endif

    shutdown();

    return 0;
}
