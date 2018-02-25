#if defined(WINVER)
#include <windows.h>
#endif


#include <string>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

enum class ESoundDriver {
    kNone, kPcSpeaker, kOpl2Lpt, kTandy, kCovox
};

ESoundDriver soundDriver = ESoundDriver::kNone;
void initOPL2();
void playTune(const std::string &);

///game interface
void init();
void loopTick();
void shutdown();
bool  isPlaying();
///


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
            initOPL2();
            playTune("t200i101o1a");
        }
    }

#endif

#endif

    init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loopTick, 20, 1);
#else
    while (isPlaying()) {
        loopTick();
    }
#endif

    shutdown();

    return 0;
}
