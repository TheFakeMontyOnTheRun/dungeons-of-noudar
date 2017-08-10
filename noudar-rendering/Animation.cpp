//
// Created by monty on 15-03-2017.
//
#include "glm/glm.hpp"
#include <memory>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;


#include <string>
#include "GraphicNode.h"
#include "AnimationStep.h"
#include "Animation.h"

namespace odb {
    Animation::Animation(vector<AnimationStep> aStepList, bool aRepeatedPlayback, std::string aNextAnimation ) : mStepList(aStepList), mRepeatedPlayback(aRepeatedPlayback), mNextAnimation(aNextAnimation) {
    }
}
