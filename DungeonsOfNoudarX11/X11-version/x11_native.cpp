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
#include <iostream>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;

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

Display *display;
Window window;
EGLDisplay eglDisplay;
EGLSurface eglSurface;


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
    XVisualInfo *visInfo, visTemplate;
    int num_visuals;
    EGLContext ctx;
    EGLConfig config;
    EGLint num_configs;
    EGLint vid;

    scrnum = DefaultScreen(x_dpy);
    root = RootWindow(x_dpy, scrnum);

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

    /* The X window visual must match the EGL config */
    visTemplate.visualid = vid;
    visInfo = XGetVisualInfo(x_dpy, VisualIDMask, &visTemplate, &num_visuals);
    if (!visInfo) {
        printf("Error: couldn't get X visual\n");
        exit(1);
    }

    /* window attributes */
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(x_dpy, root, visInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    win = XCreateWindow(x_dpy, root, 0, 0, width, height,
                        0, visInfo->depth, InputOutput,
                        visInfo->visual, mask, &attr);

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
}

void tick() {
    gameLoopTick( 20 );
    renderFrame( 20 );
    eglSwapBuffers(eglDisplay, eglSurface);
}

void
event_loop(Display *dpy, Window win,
           EGLDisplay egl_dpy, EGLSurface egl_surf) {

    display = dpy;
    window = win;
    eglDisplay = egl_dpy;
    eglSurface = egl_surf;

    while (1) {

        XEvent event;

        if (XCheckWindowEvent(dpy, win, ExposureMask | KeyPressMask, &event)) {

            switch (event.type) {
                case KeyPress: {
                    int code;
                    code = XLookupKeysym(&event.xkey, 0);

                    if (code == XK_Left) {
                        rotateCameraLeft();
                    } else if (code == XK_Right) {
                        rotateCameraRight();
                    } else if (code == XK_Up) {
                        moveUp();
                    } else if (code == XK_Down) {
                        moveDown();
                    } else if (code == XK_braceleft || code == XK_bracketleft) {
                        pickupItem();
                    } else if (code == XK_braceright || code == XK_bracketright ) {
                        dropItem();
                    } else if (code == XK_z) {
                        moveLeft();
                    } else if (code == XK_x) {
                        moveRight();
                    } else if (code == XK_minus) {
                        cyclePrevItem();
                    } else if (code == XK_equal) {
                        cycleNextItem();
                    } else if (code == XK_space ) {
                        interact();
                    } else if (code == XK_Escape ) {
                        return;
                    }
                }
                break;
            }
        }

        tick();
    }
}

