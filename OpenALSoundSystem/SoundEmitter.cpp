//
// Created by monty on 10/10/16.
//
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <iostream>

#include "glm/glm.hpp"
#include <memory>

#include <AL/al.h>
#include <AL/alc.h>
#include <unistd.h>

#include "SoundClip.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

namespace odb {
    void playSource(void* arg) {
        ALuint source = static_cast<ALuint>(reinterpret_cast<intptr_t>(arg));
        alSourcePlay(source);
    }


    SoundEmitter::SoundEmitter( std::shared_ptr<SoundClip> aSample ) : mSample(aSample) {
        ALuint sources[1];
        alGenSources(1, sources);
        mEmitterHandle = sources[ 0 ];
        alSourcei(mEmitterHandle, AL_BUFFER, aSample->mBufferHandle );

        std::cout << "is source?" << (alIsSource( mEmitterHandle ) == AL_TRUE)<< std::endl;

        ALint state;
        alGetSourcei(mEmitterHandle, AL_SOURCE_STATE, &state);

        std::cout << "state: " << state << std::endl;

    }

    void SoundEmitter::play(std::shared_ptr<SoundListener> listener) {
        ALfloat listenerPos[] = {0.0, 0.0, 0.0};
        ALfloat listenerVel[] = {0.0, 0.0, 0.0};
        ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

        alListenerfv(AL_POSITION, listenerPos);
        alListenerfv(AL_VELOCITY, listenerVel);
        alListenerfv(AL_ORIENTATION, listenerOri);
        alListenerf(AL_GAIN, mVolume);

        std::cout << "Playing:" << std::endl;

#ifdef __EMSCRIPTEN__
        emscripten_async_call(playSource, reinterpret_cast<void*>(mEmitterHandle), 700);
#else
        usleep(700000);
        playSource(reinterpret_cast<void*>(mEmitterHandle));
#endif

        ALint state;
        alGetSourcei(mEmitterHandle, AL_SOURCE_STATE, &state);

        std::cout << "state: " << state << std::endl;

    }
}

