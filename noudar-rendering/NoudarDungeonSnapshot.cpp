//
// Created by monty on 06/12/16.
//
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <map>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <array>
#include <stdio.h>
#include <cmath>

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"

#include "NoudarDungeonSnapshot.h"
namespace odb {
    std::ostream &operator<<(std::ostream &os, const NoudarDungeonSnapshot& aSnapshot) {

        for ( int y = 0; y < Knights::kMapSize; ++y ) {
            for ( int x = 0; x < Knights::kMapSize; ++x ) {

                char element = '.';

                if ( aSnapshot.mVisibilityMap[ y ][ x ] == EVisibility::kVisible ) {
                    if ( aSnapshot.snapshot[ y ][ x ] != EActorsSnapshotElement::kNothing ) {
                        element = '^';
                    } else {
                        element = aSnapshot.map[ y ][ x ];
                    }
                }

                os << element;
            }
            os << std::endl;
        }

        return os;
    }

    std::string to_string(const NoudarDungeonSnapshot& aSnapshot) {
        std::stringstream ss;
        ss << aSnapshot;
        return ss.str();
    }
}
