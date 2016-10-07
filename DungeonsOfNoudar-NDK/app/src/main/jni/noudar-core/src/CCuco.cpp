#include <vector>
#include <memory>
#include "Vec2i.h"
#include "IMapElement.h"
#include "CActor.h"
#include "CCuco.h"
#include "CMap.h"

const int DEFAULT_AP = 7;
namespace Knights {

    CCuco::CCuco(int aId) : CActor(aId, DEFAULT_AP) {
        mView = '@';
        mTeam = ETeam::kVillains;
        mHP = 5;
        mAttack = 4;
        mDefence = 1;
    }

    bool CCuco::dealWith( std::shared_ptr<CMap> map, int x, int y ) {
            int dx = x - mPosition.x;
            int dy = y - mPosition.y;

            std::shared_ptr<CCuco> sharedThis = shared_from_this();

            if ( abs( dx ) > abs( dy ) ) {

                    if ( dx < 0 ) {
                            map->move( EDirection::kWest, sharedThis );
                            return true;
                    } else if ( dx > 0 ) {
                            map->move( EDirection::kEast, sharedThis );
                            return true;
                    }
            } else {

                    if ( dy < 0 ) {
                            map->move( EDirection::kNorth, sharedThis );
                            return true;
                    } else if (dy > 0 ){
                            map->move( EDirection::kSouth, sharedThis );
                            return true;
                    }
            }

            return false;
    }

    bool CCuco::actOn( int newX, int newY,  std::shared_ptr<CMap> map ) {

            if (map->isValid( newX, newY ) ) {

                    auto otherActor = map->getActorAt( Vec2i{ newX, newY } );

                    if (otherActor != nullptr
                        && otherActor->getTeam() == ETeam::kHeroes) {

                            if (dealWith( map, newX, newY ) ) {
                                    return true;
                            }
                    }

                    if ( map->isBlockAt( newX, newY ) ) {
                            return true;
                    }

            }
            return false;
    }

    void CCuco::update( std::shared_ptr<CMap> map ) {

            Vec2i scan;
            int newX;
            int newY;

            for (int x = 0; x < 10; ++x) {

                    newX =  (x + mPosition.x);
                    newY =  (mPosition.y);
                    scan.x = newX;
                    scan.y = newY;
                    if ( actOn( newX, newY, map ) ) {
                            break;
                    }
            }

            for (int x = 0; x > -10; --x) {

                    newX =  (x + mPosition.x);
                    newY =  (mPosition.y);
                    scan.x = newX;
                    scan.y = newY;

                    if ( actOn( newX, newY, map ) ) {
                            break;
                    }
            }

            for (int y = 0; y < 10; ++y) {

                    newX = (mPosition.x);
                    newY = (y + mPosition.y);
                    scan.x = newX;
                    scan.y = newY;

                    if ( actOn( newX, newY, map ) ) {
                            break;
                    }
            }

            for (int y = 0; y > -10; --y) {

                    newX = (mPosition.x);
                    newY = (y + mPosition.y);
                    scan.x = newX;
                    scan.y = newY;

                    if ( actOn( newX, newY, map ) ) {
                            break;
                    }
            }
    }

    void CCuco::endOfTurn() {

    }
}