#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <string.h>
#include <memory>
#include <iterator>
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

#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "IFileLoaderDelegate.h"
#include "CPlainFileLoader.h"
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
#include "WindowOperations.h"
#include "x11.h"
#include "Common.h"
#include "LoadPNG.h"
#include "Logger.h"

const int winWidth = 640, winHeight = 480;

Display *x_dpy;
Window win;
EGLSurface egl_surf;
EGLContext egl_ctx;
EGLDisplay egl_dpy;
EGLint egl_major, egl_minor;
const char *s;


std::vector <std::shared_ptr<odb::NativeBitmap>> loadTextures( std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader) {
    std::vector<std::shared_ptr<odb::NativeBitmap>> toReturn;

    toReturn.push_back( loadPNG( "grass.png", fileLoader) );
    toReturn.push_back( loadPNG( "stonefloor.png", fileLoader) );
    toReturn.push_back( loadPNG( "bricks.png", fileLoader) );
    toReturn.push_back( loadPNG( "arch.png", fileLoader) );
    toReturn.push_back( loadPNG( "bars.png", fileLoader) );
    toReturn.push_back( loadPNG( "begin.png", fileLoader) );
    toReturn.push_back( loadPNG( "exit.png", fileLoader) );
    toReturn.push_back( loadPNG( "bricks_blood.png", fileLoader) );
    toReturn.push_back( loadPNG( "bricks_candles.png", fileLoader) );
    toReturn.push_back( loadPNG( "foe0.png", fileLoader) );
    toReturn.push_back( loadPNG( "foe1.png", fileLoader) );
    toReturn.push_back( loadPNG( "foe2.png", fileLoader) );
    toReturn.push_back( loadPNG( "foe3.png", fileLoader) );
    toReturn.push_back( loadPNG( "foe4.png", fileLoader) );
    toReturn.push_back( loadPNG( "foe5.png", fileLoader) );
    toReturn.push_back( loadPNG( "crusader0.png", fileLoader) );
    toReturn.push_back( loadPNG( "crusader1.png", fileLoader) );
    toReturn.push_back( loadPNG( "crusader2.png", fileLoader) );
    toReturn.push_back( loadPNG( "shadow.png", fileLoader) );
    toReturn.push_back( loadPNG( "ceiling.png", fileLoader) );
    toReturn.push_back( loadPNG( "ceilingdoor.png", fileLoader) );
    toReturn.push_back( loadPNG( "ceilingbegin.png", fileLoader) );
    toReturn.push_back( loadPNG( "ceilingend.png", fileLoader) );
    toReturn.push_back( loadPNG( "splat0.png", fileLoader) );
    toReturn.push_back( loadPNG( "splat1.png", fileLoader) );
    toReturn.push_back( loadPNG( "splat2.png", fileLoader) );
    toReturn.push_back( loadPNG( "ceilingbars.png", fileLoader) );
    toReturn.push_back( loadPNG( "clouds.png", fileLoader) );
    toReturn.push_back( loadPNG( "stonegrassfar.png", fileLoader) );
    toReturn.push_back( loadPNG( "grassstonefar.png", fileLoader) );
    toReturn.push_back( loadPNG( "stonegrassnear.png", fileLoader) );
    toReturn.push_back( loadPNG( "grassstonenear.png", fileLoader) );
    toReturn.push_back( loadPNG( "cross.png", fileLoader) );
    toReturn.push_back( loadPNG( "shotgun0.png", fileLoader) );
    toReturn.push_back( loadPNG( "shotgun1.png", fileLoader) );

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
    std::string gVertexShader = Knights::readToString(fd);
    fclose(fd);

    fd = fopen("res/fragment.glsl", "r");
    std::string gFragmentShader = Knights::readToString(fd);
    fclose(fd);

	auto fileLoader = std::make_shared<Knights::CPlainFileLoader>("res/");

	std::istringstream meshListData( fileLoader->loadFileFromPath( "mesh.list" ) );

	std::vector<std::string> meshList{std::istream_iterator<std::string>(meshListData),
	                                   std::istream_iterator<std::string>{}};

	setupGraphics(winWidth, winHeight, gVertexShader, gFragmentShader, loadTextures(fileLoader), fileLoader);

    auto soundListener = std::make_shared<odb::SoundListener>();

    std::vector<std::shared_ptr<odb::SoundEmitter>> sounds;

    std::string filenames[] {
            "res/grasssteps.wav",
            "res/stepsstones.wav",
            "res/bgnoise.wav",
            "res/monsterdamage.wav",
            "res/monsterdead.wav",
            "res/playerdamage.wav",
            "res/playerdead.wav",
            "res/swing.wav"
    };

    for ( auto filename : filenames ) {
        FILE *file = fopen( filename.c_str(), "r");
        auto soundClip = odb::makeSoundClipFrom( file );

        sounds.push_back( std::make_shared<odb::SoundEmitter>(soundClip) );
    }

    setSoundEmitters( sounds, soundListener );
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
