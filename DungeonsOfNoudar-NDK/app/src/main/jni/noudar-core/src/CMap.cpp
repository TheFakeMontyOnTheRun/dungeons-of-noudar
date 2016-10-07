#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "Vec2i.h"
#include "IMapElement.h"
#include "CDoorway.h"
#include "CActor.h"
#include "CMap.h"
#include "CBullKnight.h"
#include "CFalconKnight.h"
#include "CTurtleKnight.h"
#include "CCuco.h"

#include <iostream>

namespace Knights {
    void CMap::endOfTurn() {
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {
                if (mActors[y][x] != nullptr) {
                    mActors[y][x]->endOfTurn();
                }
            }
        }

        actors.erase( std::remove_if( actors.begin(), actors.end(),
                                           [](std::shared_ptr<CActor> actor){ return !actor->isAlive();}
        ), actors.end() );
    }


    CMap::CMap(const std::string &mapData) {

        char element;
        std::shared_ptr<CActor> actor = nullptr;
        int id = 0;
        for (int y = 0; y < 20; ++y) {
            for (int x = 0; x < 20; ++x) {

                element = mapData[(y * 20) + x];
                block[y][x] = false;
                map[y][x] = nullptr;
                mElement[ y ][ x ] = element;

                switch (element) {
                    case '0':
                    case '1':
                    case '#':
                    case '/':
                    case '\\':
                    case '|':
                        block[y][x] = (element != '0');
                        break;
                    case '~':
                        block[y][x] = false;
                        break;

                    case '4':
                        actor = mAvatar = std::make_shared<CBullKnight>(id++);
                        mElement[ y ][ x ] = '.';
                        break;
                    case '9':
                    case '*':
                        map[y][x] = std::make_shared<CDoorway>(element == '9' ? EDoorwayFunction::kExit : EDoorwayFunction::kEntry);
                        break;
                    case '5':
                    case '6':
                        actor = std::make_shared<CCuco>(id++);
                        mElement[ y ][ x ] = '.';
                        break;
                }

                if (actor != nullptr) {
                    actors.push_back(actor);
                    mActors[y][x] = actor;
                    actor->setPosition( { x, y } );
                    actor = nullptr;
                }
            }
        }
    }


    std::shared_ptr<CActor> CMap::attack(std::shared_ptr<CActor> actor, Vec2i position, bool mutual) {

        std::shared_ptr<CActor> otherActor = getActorAt( position );

        if ( otherActor == nullptr ) {
            return nullptr;
        }

        if (actor->getTeam() != otherActor->getTeam()) {
            actor->performAttack(otherActor);

            if (mutual) {
                otherActor->performAttack(actor);
            }

            if (!actor->isAlive() ) {
                auto position = actor->getPosition();
                mActors[position.y][position.x] = nullptr;
            }

            if (!otherActor->isAlive()) {
                auto position = otherActor->getPosition();
                mActors[position.y][position.x] = nullptr;
            }
        }

        return otherActor;
    }


    bool CMap::attackIfNotFriendly(EDirection d, std::shared_ptr<CActor> actor, bool mutual) {

        std::shared_ptr<CActor> other = nullptr;

        auto position = actor->getPosition();

        switch (d) {

            case EDirection::kEast:
                other = attack(actor, Vec2i{position.x + 1, position.y}, mutual);
                break;

            case EDirection::kWest:
                other = attack(actor, Vec2i{position.x - 1, position.y}, mutual);
                break;

            case EDirection::kSouth:
                other = attack(actor, Vec2i{position.x, position.y + 1}, mutual);
                break;

            case EDirection::kNorth:
                other = attack(actor, Vec2i{position.x, position.y - 1}, mutual);
                break;
        }

        return (other != nullptr);
    }


    void CMap::move(EDirection d, std::shared_ptr<CActor> actor) {

        if (actor->canAttack() && attackIfNotFriendly(d, actor, true)) {
            return;
        }

        if (!actor->canMove()) {
            return;
        }


        bool moved = false;

        auto position = actor->getPosition();

        switch (d) {

            case EDirection::kEast:

                if (!isBlockAt(position.x + 1, position.y ) ) {
                    moved = true;
                    moveActor( position, { position.x + 1, position.y }, actor );
                }
                break;

            case EDirection::kWest:
                if (!isBlockAt(position.x - 1, position.y ) ) {
                    moved = true;
                    moveActor( position, { position.x - 1, position.y }, actor );
                }
                break;

            case EDirection::kSouth:
                if (!isBlockAt(position.x, position.y + 1 ) ) {
                    moved = true;
                    moveActor( position, { position.x, position.y + 1 }, actor );
                }
                break;

            case EDirection::kNorth:
                if (!isBlockAt(position.x, position.y - 1) ) {
                    moved = true;
                    moveActor( position, { position.x, position.y - 1}, actor );
                }
                break;

        }

        if (moved) {
            actor->onMove();
        }
    }

    bool CMap::isValid(int x, int y) {
        if ( x < 0 || x > 20 || y < 0 || y > 20 ) {
            return false;
        }
        return true;
    }

    bool CMap::isBlockAt(int x, int y) {

        if ( !isValid( x, y ) ) {
            return true;
        }

        if ( mActors[ y ][ x ] != nullptr ) {
            return true;
        }

        return block[ y ][ x ];
    }

    std::shared_ptr<CActor> CMap::getActorAt( Vec2i position ) {
        return mActors[ position.y ][ position.x ];
    }

    char CMap::getElementAt( int x, int y ) {
        return mElement[ y ][ x ];
    }

    std::vector<std::shared_ptr<CActor>> CMap::getActors() {
        return actors;
    }

    std::shared_ptr<CActor> CMap::getAvatar() {
        return mAvatar;
    }

    void CMap::setActorAt(Vec2i position, std::shared_ptr<CActor> actor) {
        mActors[position.y][position.x] = actor;
    }

    void CMap::moveActor(Vec2i from, Vec2i to, std::shared_ptr<CActor> actor) {
        mActors[from.y][from.x] = nullptr;
        mActors[to.y][to.x] = actor;
        actor->setPosition( to );
    }
}