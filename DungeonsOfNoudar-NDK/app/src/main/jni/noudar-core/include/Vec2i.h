//
// Created by monty on 05/10/16.
//

#ifndef KNIGHTS_IN_PORTO_VEC2I_H
#define KNIGHTS_IN_PORTO_VEC2I_H
namespace Knights {
    class Vec2i {
    public:
        int x = 0;
        int y = 0;

        Vec2i( int aX, int aY ) {
            x = aX;
            y = aY;
        }

        Vec2i() { x = y = 0; }

        bool operator==( const Vec2i &other ) {
            return this->x == other.x && this->y == other.y;
        }
    };
}
#endif //KNIGHTS_IN_PORTO_VEC2I_H
