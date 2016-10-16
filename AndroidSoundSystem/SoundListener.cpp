//
// Created by monty on 15/10/16.
//

#include <stdio.h>
#include <memory>
#include "glm/glm.hpp"

#include "AudioSink.h"
#include "SoundListener.h"

odb::SoundListener::SoundListener(std::shared_ptr<AudioSink> audioSink) : mAudioSink( audioSink ) {

}

