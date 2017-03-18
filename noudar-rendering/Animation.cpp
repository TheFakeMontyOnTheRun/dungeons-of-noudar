//
// Created by monty on 15-03-2017.
//
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include "GraphicNode.h"
#include "AnimationStep.h"
#include "Animation.h"

namespace odb {
    Animation::Animation(std::vector<AnimationStep> aStepList, bool aRepeatedPlayback ) : mStepList(aStepList), mRepeatedPlayback(aRepeatedPlayback) {
    }
}
