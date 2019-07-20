#include <iostream>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assert.h>
#include <math.h>
#include <map>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <iostream>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>


#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <iostream>
#include <emscripten.h>
#include <vector>
#include <array>

using std::vector;
using std::array;




#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "IFileLoaderDelegate.h"
#include "x11.h"
#include "NativeBitmap.h"

#include "Vec2i.h"
#include "NativeBitmap.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"

#include "NoudarDungeonSnapshot.h"

#include "GameNativeAPI.h"


#include <emscripten.h>
#include <emscripten/html5.h>


Display *display;
Window window;
EGLDisplay eglDisplay;
EGLSurface eglSurface;

extern "C" {

EM_BOOL keydown_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {

    std::string code = e->code;

    if (code == "ArrowLeft") {
        rotateCameraLeft();
    } else if (code == "ArrowRight") {
        rotateCameraRight();
    } else if (code == "ArrowUp") {
        moveUp();
    } else if (code == "ArrowDown") {
        moveDown();
    } else if (code == "KeyZ") {
        moveLeft();
    } else if (code == "KeyX") {
        moveRight();
    } else if (code == "Space") {
        interact();
    } else if (code == "Equal") {
        cycleNextItem();
    } else if (code == "Minus") {
        cyclePrevItem();
    } else if (code == "BracketRight") {
        pickupItem();
    } else if (code == "Backslash") {
        dropItem();
    }


    std::cout << "key code: " << e->code << std::endl;

    return true;
}
EM_BOOL keypress_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    return true;
}
EM_BOOL keyup_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    return true;
}
}


void enterFullScreenMode() {
    EmscriptenFullscreenStrategy s;
    memset(&s, 0, sizeof(s));
    s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
    s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
    s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    emscripten_enter_soft_fullscreen(0, &s);
}

/*
 * Create an RGB, double-buffered X window.
 * Return the window and context handles.
 */
extern void make_x_window(Display *x_dpy, EGLDisplay egl_dpy,
                          const char *name,
                          int x, int y, int width, int height,
                          Window *winRet,
                          EGLContext *ctxRet,
                          EGLSurface *surfRet) {
    static const EGLint attribs[] = {
            EGL_RED_SIZE, 1,
            EGL_GREEN_SIZE, 1,
            EGL_BLUE_SIZE, 1,
            EGL_DEPTH_SIZE, 1,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };

    static const EGLint ctx_attribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };


    int scrnum;
    XSetWindowAttributes attr;
    unsigned long mask;
    Window root;
    Window win;
    //   XVisualInfo *visInfo, visTemplate;

    EGLContext ctx;
    EGLConfig config;
    EGLint num_configs;
    EGLint vid;

    scrnum = DefaultScreen(x_dpy);
    root = DefaultRootWindow(x_dpy);

    if (!eglChooseConfig(egl_dpy, attribs, &config, 1, &num_configs)) {
        printf("Error: couldn't get an EGL visual config\n");
        exit(1);
    }

    assert(config);
    assert(num_configs > 0);

    if (!eglGetConfigAttrib(egl_dpy, config, EGL_NATIVE_VISUAL_ID, &vid)) {
        printf("Error: eglGetConfigAttrib() failed\n");
        exit(1);
    }

    /* window attributes */
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    // attr.colormap = XCreateColormap( x_dpy, root, visInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    win = XCreateWindow(x_dpy, root, 0, 0, width, height,
                        0, CopyFromParent, InputOutput,
                        CopyFromParent, mask, &attr);

    /* set hints and properties */
    {
        XSizeHints sizehints;
        sizehints.x = x;
        sizehints.y = y;
        sizehints.width = width;
        sizehints.height = height;
        sizehints.flags = USSize | USPosition;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    ctx = eglCreateContext(egl_dpy, config, EGL_NO_CONTEXT, ctx_attribs);
    if (!ctx) {
        printf("Error: eglCreateContext failed\n");
        exit(1);
    }


    /* test eglQueryContext() */
    {
        EGLint val;
        eglQueryContext(egl_dpy, ctx, EGL_CONTEXT_CLIENT_VERSION, &val);
        assert(val == 2);
    }


    *surfRet = eglCreateWindowSurface(egl_dpy, config, win, NULL);
    if (!*surfRet) {
        printf("Error: eglCreateWindowSurface failed\n");
        exit(1);
    }

    /* sanity checks */
    {
        EGLint val;
        eglQuerySurface(egl_dpy, *surfRet, EGL_WIDTH, &val);
        assert(val == width);
        eglQuerySurface(egl_dpy, *surfRet, EGL_HEIGHT, &val);
        assert(val == height);
        assert(eglGetConfigAttrib(egl_dpy, config, EGL_SURFACE_TYPE, &val));
        assert(val & EGL_WINDOW_BIT);
    }

    *winRet = win;
    *ctxRet = ctx;
    EM_ASM(
        document.title = "Dungeons of Noudar"
    );

    enterFullScreenMode();
}

void tick() {
    gameLoopTick( 16 );
    renderFrame( 16 );
    eglSwapBuffers(eglDisplay, eglSurface);
}

void
event_loop(Display *dpy, Window win,
           EGLDisplay egl_dpy, EGLSurface egl_surf) {

    display = dpy;
    window = win;
    eglDisplay = egl_dpy;
    eglSurface = egl_surf;

    if (emscripten_set_keydown_callback("#document", nullptr, true, keydown_callback) == EMSCRIPTEN_RESULT_SUCCESS) {
        std::cout << "down ok" << std::endl;
    } else {
        std::cout << "down not ok" << std::endl;
    }

    if (emscripten_set_keypress_callback("#document", nullptr, true, keypress_callback) == EMSCRIPTEN_RESULT_SUCCESS) {
        std::cout << "press ok" << std::endl;
    } else {
        std::cout << "press not ok" << std::endl;
    }

    if (emscripten_set_keyup_callback("#document", nullptr, true, keyup_callback) == EMSCRIPTEN_RESULT_SUCCESS) {
        std::cout << "up ok" << std::endl;
    } else {
        std::cout << "up not ok" << std::endl;
    }

    emscripten_set_main_loop(tick, 60, 1);
}
