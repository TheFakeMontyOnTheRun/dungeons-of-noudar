//
// Created by monty on 15-03-2017.
//

#ifndef DUNGEONS_OF_NOUDAR_X11_GRAPHICNODE_H
#define DUNGEONS_OF_NOUDAR_X11_GRAPHICNODE_H

namespace odb {
    class GraphicNode {
    public:
        explicit GraphicNode(std::string aFrameId, glm::vec2 aRelativePosition, glm::vec2 aFinalPosition );
        explicit GraphicNode(std::string aFrameId, glm::vec2 aRelativePosition );
        std::string mFrameId;
        glm::vec2 mRelativePosition;
        glm::vec2 mFinalPosition;

        glm::vec2 getPositionForTime( float progress );
    };
}

#endif //DUNGEONS_OF_NOUDAR_X11_GRAPHICNODE_H
