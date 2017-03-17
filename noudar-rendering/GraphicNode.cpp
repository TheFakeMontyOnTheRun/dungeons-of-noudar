//
// Created by monty on 15-03-2017.
//
#include <string>
#include <glm/glm.hpp>
#include "GraphicNode.h"

namespace odb {
    GraphicNode::GraphicNode(std::string aFrameId, glm::vec2 aRelativePosition ) :
    mFrameId(aFrameId), mRelativePosition(aRelativePosition){
    }
}

