#include "Vec2i.h"
#include "IMapElement.h"

namespace Knights {
    IMapElement::IMapElement() : mPosition(0,0), 
    mView( kEmptySpace ), mIsBlocker( false ) {
    }

    Vec2i IMapElement::getPosition() {
        return mPosition;
    }

    bool IMapElement::isBlocker() {
        return mIsBlocker;
    }

    char IMapElement::getView() {
        return mView;
    }
}