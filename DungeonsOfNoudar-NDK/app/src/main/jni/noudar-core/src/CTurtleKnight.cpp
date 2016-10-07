#include <memory>
#include <vector>
#include "Vec2i.h"
#include "IMapElement.h"
#include "CActor.h"
#include "CTurtleKnight.h"
#include "CMap.h"

const int DEFAULT_AP = 4;

namespace Knights {
    CTurtleKnight::CTurtleKnight(int aId) : CActor(aId, DEFAULT_AP) {
        mView = '^';
        mTeam = ETeam::kHeroes;
        mHP = 30;
        mAttack = 2;
        mDefence = 5;
    }

    void CTurtleKnight::update(std::shared_ptr <CMap> map) {

    }
}
