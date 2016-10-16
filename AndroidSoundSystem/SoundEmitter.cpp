//
// Created by monty on 15/10/16.
//

#include <iostream>

#include "glm/glm.hpp"
#include <memory>
#include "AudioSink.h"
#include "SoundClip.h"
#include "SoundListener.h"

#include "SoundEmitter.h"

void odb::SoundEmitter::play(std::shared_ptr<SoundListener> listener) {
	mAudioSink->play( mEmitterHandle, 0.5f, 0.75f );
}

odb::SoundEmitter::SoundEmitter(std::shared_ptr<AudioSink> audioSink, std::shared_ptr<SoundClip> aSample) : mAudioSink( audioSink ) {
	mEmitterHandle = aSample->mBufferHandle;
}



