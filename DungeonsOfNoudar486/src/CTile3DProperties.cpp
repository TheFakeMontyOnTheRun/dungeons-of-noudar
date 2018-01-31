//
// Created by monty on 14/01/17.
//
#include <cctype>
#include <iterator>
#include <functional>
#include <memory>
#include <string>
#include <iterator>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <sg14/fixed_point>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using sg14::fixed_point;
using eastl::vector;
using eastl::array;

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "NativeBitmap.h"
#include "Logger.h"
#include "RasterizerCommon.h"
#include "CTile3DProperties.h"

namespace odb {

    GeometryType parseGeometryType(std::string token);

    CTile3DProperties readPropertiesLine(vector<std::string>::iterator& pos) {

        CTile3DProperties properties;

        pos = std::next(pos);
        properties.mNeedsAlphaTest = ((*pos)[0] == '1');
        pos = std::next(pos);
        properties.mCeilingTexture = *pos;
        pos = std::next(pos);
        properties.mFloorTexture = *pos;
        pos = std::next(pos);
        properties.mMainWallTexture = *pos;
        pos = std::next(pos);
        properties.mGeometryType = parseGeometryType( *pos );
        pos = std::next(pos);
        properties.mCeilingRepeatedWallTexture = *pos;
        pos = std::next(pos);
        properties.mFloorRepeatedWallTexture = *pos;
        pos = std::next(pos);
        properties.mCeilingRepetitions = std::atoi( pos->c_str() );
        pos = std::next(pos);
        properties.mFloorRepetitions = std::atoi( pos->c_str() );
        pos = std::next(pos);
        properties.mCeilingHeight = FixP{atof( pos->c_str() )};
        pos = std::next(pos);
        properties.mFloorHeight = FixP{atof( pos->c_str() )};

        return properties;
    }

    GeometryType parseGeometryType(std::string token) {

        if ( token == "cube") {
            return kCube;
        }

        if ( token == "leftfar") {
            return kRightNearWall;
        }

        if ( token == "leftnear") {
            return kLeftNearWall;
        }

        return kNone;
}


    CTilePropertyMap CTile3DProperties::parsePropertyList(std::string propertyFile) {

        odb::CTilePropertyMap map;
        vector<std::string> tokens;
        int line = 0;

        auto ptr = std::begin(propertyFile);
        auto end = std::end(propertyFile);
        auto lineBegin = ptr;

        while ( ptr != end ) {
            std::string tmp;
            if ( !std::isspace(*ptr) ) {
                auto pos2 = ptr;

                while (pos2 != end && !std::isspace(*pos2) ) {
                    tmp += *pos2;
                    ++pos2;
                }

                if ( !tmp.empty()) {
                    tokens.push_back(tmp);
                }

                ptr = pos2;
            }

            if ( ptr != end ) {

                if ( *ptr == '\n' ) {
                    CTileId id = *lineBegin;
                    auto tokensBegin = std::begin(tokens);
                    auto props = readPropertiesLine(tokensBegin);
                    tokens.clear();
                    map[id] = props;
                    lineBegin = ptr + 1;
                }

                ++ptr;

            }
        }

        return map;
    }
}

