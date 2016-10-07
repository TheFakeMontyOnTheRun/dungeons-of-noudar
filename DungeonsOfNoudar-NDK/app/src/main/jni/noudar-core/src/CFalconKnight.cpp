#include <memory>
#include <vector>
#include "Vec2i.h"
#include "IMapElement.h"
#include "CActor.h"
#include "CFalconKnight.h"
#include "CMap.h"

const int DEFAULT_AP = 6;

namespace Knights {

    CFalconKnight::CFalconKnight(int aId) : CActor(aId, DEFAULT_AP) {
        mView = '&';
        mTeam = ETeam::kHeroes;
        mHP = 2;
        mAttack = 3;
        mDefence = 3;
    }

    void CFalconKnight::update(std::shared_ptr <CMap> map) {

    }
}