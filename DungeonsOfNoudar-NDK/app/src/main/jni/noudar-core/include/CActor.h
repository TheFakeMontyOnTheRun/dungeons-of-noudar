// CActor - a generic abstract Actor
// 2016 - Daniel Monteiro
//
#ifndef knights2_CActor_H
#define knights2_CActor_H

namespace Knights {
    enum class EStance {
        kStanding,
        kAttacking,
        kDead
    };

    enum class ETeam {
        kHeroes,
        kVillains
    };

    enum class EDirection {
        kNorth,
        kEast,
        kSouth,
        kWest
    };

    class CMap;

    class CActor {
    protected:
        EStance mStance;
        EDirection mDirection;
        ETeam mTeam;
        int mHP;
        int mDefence;
        int mAttack;
        int mRemainingAP;
        int mDefaultAP;
        int mId;
        char mView;
        Vec2i mPosition;
    public:
        void performAttack( std::shared_ptr<CActor> other);
        CActor( int aId, int defaultAP );
        Vec2i getPosition();
        void setPosition( Vec2i position );
        void onMove();
        void onAttack();
        bool canMove();
        bool canAttack();
        bool hasEnoughAP();
        bool isAlive();
        void turnLeft();
        void turnRight();

        int getId();
        int getHP();
        int getAP();
        int getDefense();
        int getAttack();

        ETeam getTeam();
        EDirection getDirection();
        virtual void update( std::shared_ptr<CMap> map ) {}
        virtual void endOfTurn();
        virtual ~CActor() = default;
    };
}
#endif
