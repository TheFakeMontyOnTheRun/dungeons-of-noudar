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
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>

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
#include "Common.h"
#include "LoadPNG.h"

const int winWidth = 640, winHeight = 480;
SDL_Surface* video;
bool done = false;
bool isActive = false;
SDL_Event event;
const SDL_VideoInfo *videoInfo;
int videoFlags;

void handleKeyPress( SDL_keysym *keysym ) {
    switch ( keysym->sym ) 	{
	case SDLK_ESCAPE:
	    	SDL_Quit( );
	    break;
	case SDLK_F1:
	    SDL_WM_ToggleFullScreen( video );
	    break;
	case SDLK_LEFT:
		rotateCameraLeft();
		break;
	case SDLK_RIGHT:
		rotateCameraRight();
		break;
	case SDLK_UP:
		moveUp();
		break;
	case SDLK_DOWN:
		moveDown();
		break;
	case SDLK_SPACE:
		interact();
		break;
	case SDLK_z:
		moveLeft();
		break;
	case SDLK_x:
		moveRight();
		break;
    case SDLK_MINUS:
      cyclePrevItem();
      break;
    case SDLK_EQUALS:
      cycleNextItem();
      break;

    case SDLK_LEFTBRACKET:
      pickupItem();
      break;
      
    case SDLK_RIGHTBRACKET:
      dropItem();
      break;

    default:
	    break;
	}

    return;
}


void initWindow() {

	SDL_Init( SDL_INIT_VIDEO );
	
	videoFlags  = SDL_OPENGL;
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;
	videoFlags |= SDL_RESIZABLE;

	    videoInfo = SDL_GetVideoInfo( );


	if ( videoInfo->hw_available ) {
		videoFlags |= SDL_HWSURFACE;
	} else {
		videoFlags |= SDL_SWSURFACE;
	}

	if ( videoInfo->blit_hw ) {
		videoFlags |= SDL_HWACCEL;
	}

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	video = SDL_SetVideoMode( winWidth, winHeight, 0,
				videoFlags );
    
	auto gVertexShader = "";
	auto gFragmentShader = "";

	setupGraphics(winWidth, winHeight, gVertexShader, gFragmentShader, std::make_shared<Knights::CPlainFileLoader>());

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

void tick() {
    gameLoopTick( 20 );
    renderFrame( 20 );
    SDL_GL_SwapBuffers();
}

void setMainLoop() {
	while ( !done ) {
		while ( SDL_PollEvent( &event ) ) {
			switch( event.type ) {
				case SDL_ACTIVEEVENT:
					isActive = event.active.gain != 0;
				case SDL_VIDEORESIZE:
				    video = SDL_SetVideoMode( event.resize.w,
								event.resize.h,
								16, videoFlags );
				    break;
				case SDL_KEYDOWN:
				    handleKeyPress( &event.key.keysym );
				    break;
				case SDL_QUIT:
				    done = true;
				    break;
				default:
				    break;
				}
			}

	      tick();
	}
}


void destroyWindow() {
   shutdown();
}
