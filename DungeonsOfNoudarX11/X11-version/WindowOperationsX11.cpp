#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <memory>
#include <iostream>
#include <map>
#include <array>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <iostream>

#include "NativeBitmap.h"
#include "GameNativeAPI.h"
#include "WindowOperations.h"
#include "x11.h"
#include "Common.h"
#include "LoadPNG.h"

const int winWidth = 640, winHeight = 480;

Display *x_dpy;
Window win;
EGLSurface egl_surf;
EGLContext egl_ctx;
EGLDisplay egl_dpy;
EGLint egl_major, egl_minor;
const char *s;


std::vector <std::shared_ptr<odb::NativeBitmap>> loadTextures() {
    std::vector<std::shared_ptr<odb::NativeBitmap>> toReturn;

    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/grass.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/grass.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/bricks.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/arch.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/bars.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/begin.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/exit.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/bricks_blood.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/bricks_candles.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/boss0.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/boss1.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/boss2.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/cuco0.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/cuco1.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/cuco2.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/demon0.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/demon1.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/demon2.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/lady0.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/lady1.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/lady2.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/bull0.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/shadow.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/ceiling.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/ceilingdoor.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/ceilingbegin.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/ceilingend.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/splat0.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/splat1.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/splat2.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/ceilingbars.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/bricks.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 64, 64, loadPNG( "res/bricks.png", 64, 64 )));
    toReturn.push_back(std::make_shared<odb::NativeBitmap>( 128, 128, loadPNG( "res/clouds.png", 128, 128 )));

    return toReturn;
}

void initWindow() {
   x_dpy = XOpenDisplay(nullptr);

   if (!x_dpy) {
      printf("Error: couldn't open display");
      return;
   }

   egl_dpy = eglGetDisplay((EGLNativeDisplayType)x_dpy);

   if (!egl_dpy) {
      printf("Error: eglGetDisplay() failed\n");
      return;
   }

   if (!eglInitialize(egl_dpy, &egl_major, &egl_minor)) {
      printf("Error: eglInitialize() failed\n");
      return;
   }

   s = eglQueryString(egl_dpy, EGL_VERSION);
   printf("EGL_VERSION = %s\n", s);

   s = eglQueryString(egl_dpy, EGL_VENDOR);
   printf("EGL_VENDOR = %s\n", s);

   s = eglQueryString(egl_dpy, EGL_EXTENSIONS);
   printf("EGL_EXTENSIONS = %s\n", s);

   s = eglQueryString(egl_dpy, EGL_CLIENT_APIS);
   printf("EGL_CLIENT_APIS = %s\n", s);

   make_x_window(x_dpy, egl_dpy,
                 "Dungeons Of Noudar", 0, 0, winWidth, winHeight,
                 &win, &egl_ctx, &egl_surf);

   XMapWindow(x_dpy, win);

   if (!eglMakeCurrent(egl_dpy, egl_surf, egl_surf, egl_ctx)) {
      printf("Error: eglMakeCurrent() failed\n");
      return;
   }

   printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
   printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
   printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
   printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));

    FILE *fd;
    fd = fopen("res/vertex.glsl", "r");
    std::string gVertexShader = readToString(fd);
    fclose(fd);

    fd = fopen("res/fragment.glsl", "r");
    std::string gFragmentShader = readToString(fd);
    fclose(fd);

   setupGraphics(winWidth, winHeight, gVertexShader, gFragmentShader, loadTextures());
}

void setMainLoop() {
   event_loop(x_dpy, win, egl_dpy, egl_surf);
}

void destroyWindow() {
   shutdown();
   eglDestroyContext(egl_dpy, egl_ctx);
   eglDestroySurface(egl_dpy, egl_surf);
   eglTerminate(egl_dpy);
}