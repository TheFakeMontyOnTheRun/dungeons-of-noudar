//
// Created by monty on 04-07-2017.
//

#include <functional>
#include <memory>
#include <cmath>
#include <algorithm>
#include <sg14/fixed_point>
#include <vector>
#include <array>

using std::vector;
using std::array;
using sg14::fixed_point;

#include "RasterizerCommon.h"

namespace odb {

    Vec3 operator+(const Vec3& v1, const Vec3& v2 ) {
        return { v1.mX + v2.mX, v1.mY + v2.mY, v1.mZ + v2.mZ };
    }

    Vec3 operator-(const Vec3& v1, const Vec3& v2 ) {
        return { v1.mX - v2.mX, v1.mY - v2.mY, v1.mZ - v2.mZ };
    }

    Vec3& operator+=(Vec3& v1, const Vec3& v2 ) {

        v1.mX += v2.mX;
        v1.mY += v2.mY;
        v1.mZ += v2.mZ;

        return v1;
    }

    Vec3& operator-=(Vec3& v1, const Vec3& v2 ) {

        v1.mX -= v2.mX;
        v1.mY -= v2.mY;
        v1.mZ -= v2.mZ;

        return v1;
    }
}

