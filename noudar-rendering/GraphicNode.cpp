//
// Created by monty on 15-03-2017.
//
#include <string>
#include <glm/glm.hpp>
#include "GraphicNode.h"

namespace odb {
    GraphicNode::GraphicNode(std::string aFrameId, glm::vec2 aRelativePosition, glm::vec2 aFinalPosition ) :
    mFrameId(aFrameId), mRelativePosition(aRelativePosition), mFinalPosition( aFinalPosition ){
    }

    GraphicNode::GraphicNode(std::string aFrameId, glm::vec2 aRelativePosition ) :
            mFrameId(aFrameId), mRelativePosition(aRelativePosition), mFinalPosition( aRelativePosition ){
    }

    glm::vec2 GraphicNode::getPositionForTime( float progress ) {
        glm::vec2 speed = { mFinalPosition.x - mRelativePosition.x, mFinalPosition.y - mRelativePosition.y };
        return mRelativePosition + glm::vec2{ speed.x * progress, speed.y * progress };
    }
}

