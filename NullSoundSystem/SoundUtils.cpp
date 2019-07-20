//
// Created by monty on 10/10/16.
//
#include <memory>

#include "SoundClip.h"
#include "SoundUtils.h"

namespace odb {
    std::shared_ptr<SoundClip> makeSoundClipFrom(FILE *source) {
        return std::make_shared<SoundClip>(nullptr, 0, 0, 0, 0, 0);
    }
}
