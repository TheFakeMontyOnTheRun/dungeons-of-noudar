#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdio>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <map>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#ifdef __APPLE__
#if TARGET_IOS
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#endif
#else
#include <GL/gl.h>
#endif

#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;


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

const int winWidth = 640, winHeight = 480;
SDL_Window *window;
SDL_GLContext glContext;
bool done = false;
SDL_Event event;
bool windowed = false;

void handleKeyPress(SDL_Event &event) {
    auto keysym = &event.key.keysym;
    switch (keysym->sym) {
        case SDLK_ESCAPE:
	  done = true;
            break;
        case SDLK_F1:
            SDL_SetWindowFullscreen(window, windowed ? SDL_WINDOW_FULLSCREEN : 0);
            windowed = !windowed;
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
        case SDLK_e:
            cyclePrevItem();
            break;
        case SDLK_EQUALS:
        case SDLK_d:
            cycleNextItem();
            break;
            
        case SDLK_w:
        case SDLK_LEFTBRACKET:
            pickupItem();
            break;
            
        case SDLK_s:
        case SDLK_RIGHTBRACKET:
            dropItem();
            break;

        default:
            break;
    }
}


void initWindow() {

    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("The Dungeons of Noudar", 0, 0, winWidth, winHeight, SDL_WINDOW_OPENGL);
    glContext = SDL_GL_CreateContext(window);
    auto gVertexShader = "";
    auto gFragmentShader = "";

    setupGraphics(winWidth, winHeight, gVertexShader, gFragmentShader, std::make_shared<Knights::CPlainFileLoader>());

    auto soundListener = std::make_shared<odb::SoundListener>();

    vector<std::shared_ptr<odb::SoundEmitter>> sounds;

    std::string filenames[]{
            "res/grasssteps.wav",
            "res/stepsstones.wav",
            "res/bgnoise.wav",
            "res/monsterdamage.wav",
            "res/monsterdead.wav",
            "res/playerdamage.wav",
            "res/playerdead.wav",
            "res/swing.wav"
    };

    for (auto filename : filenames) {
        FILE *file = fopen(filename.c_str(), "r");
        auto soundClip = odb::makeSoundClipFrom(file);

        sounds.push_back(std::make_shared<odb::SoundEmitter>(soundClip));
      fclose(file);
    }

    setSoundEmitters(sounds, soundListener);
}

void tick() {
    gameLoopTick(20);
    renderFrame(20);
    SDL_GL_SwapWindow(window);
}

void setMainLoop() {
    while (!done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    handleKeyPress(event);
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
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
