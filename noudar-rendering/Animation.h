//
// Created by monty on 15-03-2017.
//

#ifndef DUNGEONS_OF_NOUDAR_X11_ANIMATION_H
#define DUNGEONS_OF_NOUDAR_X11_ANIMATION_H


namespace odb {

    class Animation {
    public:
        Animation(std::vector<AnimationStep> aStepList, bool aRepeatedPlayback );
        std::vector<AnimationStep> mStepList;
        bool mRepeatedPlayback;
    };
}

#endif //DUNGEONS_OF_NOUDAR_X11_ANIMATION_H
