
#include "glm/glm.hpp"
#include <memory>


#include "SoundClip.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

namespace odb {
    void playSource(void *arg) {

    }


    SoundEmitter::SoundEmitter(std::shared_ptr<SoundClip> aSample) : mSample(aSample) {

    }

    void SoundEmitter::play(std::shared_ptr<SoundListener> listener) {


    }
}

