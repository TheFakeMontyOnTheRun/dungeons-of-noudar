//
// Created by monty on 15-03-2017.
//
#include "glm/glm.hpp"
#include <memory>
#include <vector>
#include <string>
#include "GraphicNode.h"
#include "AnimationStep.h"
#include "Animation.h"

namespace odb {
    Animation::Animation(std::vector<AnimationStep> aStepList, bool aRepeatedPlayback, std::string aNextAnimation ) : mStepList(aStepList), mRepeatedPlayback(aRepeatedPlayback), mNextAnimation(aNextAnimation) {
    }
}
