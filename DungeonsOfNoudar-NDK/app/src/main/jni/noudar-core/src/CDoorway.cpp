#include "Vec2i.h"
#include "IMapElement.h"
#include "CDoorway.h"

namespace Knights {

    CDoorway::CDoorway(EDoorwayFunction f) : doorFunction(f) {
        if (f == EDoorwayFunction::kExit) {
            mView = 'E';
        } else {
            mView = 'B';
        }
    }
}