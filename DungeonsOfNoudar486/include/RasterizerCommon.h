//
// Created by monty on 27/08/16.
//

#ifndef BLANKSLATE_COMMON_H
#define BLANKSLATE_COMMON_H

namespace odb {

#ifdef LOWRES
    using FixP = fixed_point<int16_t, -4>;
#else
    using FixP = fixed_point<int32_t, -16>;
#endif


    class Vec3 {
    public:
        FixP mX;
        FixP mY;
        FixP mZ;
    };

    Vec3 operator+(const Vec3 &v1, const Vec3 &v2);

    Vec3 operator-(const Vec3 &v1, const Vec3 &v2);

    Vec3 &operator+=(Vec3 &v1, const Vec3 &v2);

    Vec3 &operator-=(Vec3 &v1, const Vec3 &v2);

    class Vec2 {
    public:
        FixP mX;
        FixP mY;
    };
}
#endif //BLANKSLATE_COMMON_H
