//
// Created by monty on 27/08/16.
//

#ifndef BLANKSLATE_COMMON_H
#define BLANKSLATE_COMMON_H

#include <Vec2i.h>

namespace odb {
    enum class ECommand {
        kUp,
        kRight,
        kDown,
        kLeft,
        kFire1,
        kFire2
    };

    using FixP = fixed_point<int32_t, -16>;

    class RayCollision {
    public:
        FixP mSquaredDistance = 0;
        Knights::Vec2i mCollisionPoint = {0,0};
        int mHeight = 0;
        int mElement = 0;
    };

    using CControlCallback = std::function< void(ECommand) >;

    int wrap360(int i);

    const int xRes = 255;
    const int yRes = 128;
    const FixP fixedYRes = FixP{128};

}
#endif //BLANKSLATE_COMMON_H
