#include <memory>
#include <utility>
#include "Vec2i.h"
#include "CActor.h"

namespace Knights {
    CActor::CActor(int aId, int defaultAP) :
            mId( aId ),
            mDefaultAP(defaultAP),
            mRemainingAP(defaultAP),
            mStance(EStance::kStanding),
            mAttack(0),
            mDefence(0),
            mHP(0),
            mDirection(EDirection::kNorth) {
    }

    bool CActor::canMove() {
        return hasEnoughAP();
    }

    bool CActor::canAttack() {
        return hasEnoughAP();
    }

    bool CActor::hasEnoughAP() {
        return mRemainingAP > 0;
    }

    void CActor::onMove() {
        --mRemainingAP;
    }

    void CActor::onAttack() {
        --mRemainingAP;
    }

    void CActor::turnLeft() {

        if ( mDirection == EDirection::kNorth ) {
            mDirection = EDirection::kWest;
        } else {
            mDirection = static_cast<EDirection>( static_cast<int>(mDirection) - 1);
        }
    }

    void CActor::turnRight() {
        if ( mDirection == EDirection::kWest ) {
            mDirection = EDirection::kNorth;
        } else {
            mDirection = static_cast<EDirection>( static_cast<int>(mDirection) + 1);
        }
    }

    void CActor::endOfTurn() {
        mRemainingAP = mDefaultAP;
    }

    void CActor::performAttack(std::shared_ptr<CActor> other) {

        onAttack();
        int diff = (mAttack - other->mDefence);

        if (diff > 0) {
            other->mHP -= diff;
        }
    }

    Vec2i CActor::getPosition() {
        return mPosition;
    }

    void CActor::setPosition(Vec2i position) {
        mPosition = position;
    }

    ETeam CActor::getTeam() {
        return mTeam;
    }

    bool CActor::isAlive() {
        return mHP > 0;
    }

    EDirection CActor::getDirection() {
        return mDirection;
    }

    int CActor::getHP() {
        return mHP;
    }

    int CActor::getAP() {
        return mRemainingAP;
    }

    int CActor::getDefense() {
        return mDefence;
    }

    int CActor::getAttack() {
        return mAttack;
    }

    int CActor::getId() {
        return mId;
    }

}