//
// Created by monty on 15-03-2017.
//

#ifndef DUNGEONS_OF_NOUDAR_X11_ANIMATIONSTEP_H
#define DUNGEONS_OF_NOUDAR_X11_ANIMATIONSTEP_H

namespace odb {
    class AnimationStep {
    public:
        vector<std::shared_ptr<odb::GraphicNode> > mNodes;
        long mDelay;
    };
}

#endif //DUNGEONS_OF_NOUDAR_X11_ANIMATIONSTEP_H
