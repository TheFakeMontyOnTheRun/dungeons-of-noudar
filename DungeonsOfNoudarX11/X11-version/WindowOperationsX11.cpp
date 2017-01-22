#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
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


std::vector <std::shared_ptr<odb::NativeBitmap>> loadTextures() {
    std::vector<std::shared_ptr<odb::NativeBitmap>> toReturn;

    toReturn.push_back( loadPNG( "res/grass.png") );
    toReturn.push_back( loadPNG( "res/stonefloor.png") );
    toReturn.push_back( loadPNG( "res/bricks.png") );
    toReturn.push_back( loadPNG( "res/arch.png") );
    toReturn.push_back( loadPNG( "res/bars.png") );
    toReturn.push_back( loadPNG( "res/begin.png") );
    toReturn.push_back( loadPNG( "res/exit.png") );
    toReturn.push_back( loadPNG( "res/bricks_blood.png") );
    toReturn.push_back( loadPNG( "res/bricks_candles.png") );
    toReturn.push_back( loadPNG( "res/foe0.png") );
    toReturn.push_back( loadPNG( "res/foe1.png") );
    toReturn.push_back( loadPNG( "res/foe2.png") );
    toReturn.push_back( loadPNG( "res/foe3.png") );
    toReturn.push_back( loadPNG( "res/foe4.png") );
    toReturn.push_back( loadPNG( "res/foe5.png") );
    toReturn.push_back( loadPNG( "res/crusader0.png") );
    toReturn.push_back( loadPNG( "res/crusader1.png") );
    toReturn.push_back( loadPNG( "res/crusader2.png") );
    toReturn.push_back( loadPNG( "res/shadow.png") );
    toReturn.push_back( loadPNG( "res/ceiling.png") );
    toReturn.push_back( loadPNG( "res/ceilingdoor.png") );
    toReturn.push_back( loadPNG( "res/ceilingbegin.png") );
    toReturn.push_back( loadPNG( "res/ceilingend.png") );
    toReturn.push_back( loadPNG( "res/splat0.png") );
    toReturn.push_back( loadPNG( "res/splat1.png") );
    toReturn.push_back( loadPNG( "res/splat2.png") );
    toReturn.push_back( loadPNG( "res/ceilingbars.png") );
    toReturn.push_back( loadPNG( "res/clouds.png"));
    toReturn.push_back( loadPNG( "res/stonegrassfar.png"));
    toReturn.push_back( loadPNG( "res/grassstonefar.png"));
    toReturn.push_back( loadPNG( "res/stonegrassnear.png"));
    toReturn.push_back( loadPNG( "res/grassstonenear.png"));
    toReturn.push_back( loadPNG( "res/cross.png"));

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

	auto fileLoader = std::make_shared<Knights::CPlainFileLoader>();
	std::vector< std::tuple<std::string, std::string, std::string >> meshes;

	{
		auto cubeMesh = fileLoader->loadFileFromPath("res/cube.obj");
		auto cubeMaterial = fileLoader->loadFileFromPath("res/cube.mtl");

		meshes.push_back(std::make_tuple("Cube", cubeMesh, cubeMaterial));
	}

	{
		auto cubeMesh = fileLoader->loadFileFromPath("res/x_victory.obj");
		auto cubeMaterial = fileLoader->loadFileFromPath("res/x_victory.mtl");

		meshes.push_back(std::make_tuple("XVictory", cubeMesh, cubeMaterial));
	}

	setMeshes( meshes );

	setupGraphics(winWidth, winHeight, gVertexShader, gFragmentShader, loadTextures());

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