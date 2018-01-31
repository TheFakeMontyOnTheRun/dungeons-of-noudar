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

    CTile3DProperties readPropertiesLine( std::string::iterator pos0, std::string::iterator pos1 ) {

        CTile3DProperties properties;
        std::string::iterator pos2;
        //the ID;
        ++pos0;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; };
        properties.mNeedsAlphaTest = ( *pos0 == '1');
        ++pos0;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mCeilingTexture = std::string{ pos0, pos2 };
        pos0 = pos2 + 1;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mFloorTexture = std::string{ pos0, pos2 };
        pos0 = pos2 + 1;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mMainWallTexture = std::string{ pos0, pos2 };
        pos0 = pos2 + 1;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mGeometryType = parseGeometryType( std::string( pos0, pos2 ) );
        pos0 = pos2 + 1;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mCeilingRepeatedWallTexture = std::string{ pos0, pos2 };
        pos0 = pos2 + 1;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mFloorRepeatedWallTexture  = std::string{ pos0, pos2 };
        pos0 = pos2 + 1;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        properties.mCeilingRepetitions = *pos0 - '0';
        pos0++;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mFloorRepetitions = *pos0 - '0';
        pos0++;


        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mCeilingHeight = FixP{atof( std::string{ pos0, pos2}.c_str() )};
        pos0 = pos2 + 1;

        while( std::isspace( *pos0 ) && pos0 != pos1 ) { ++pos0; }
        pos2 = pos0;
        while( !std::isspace( *pos2 ) && pos2 != pos1 ) { ++pos2; }
        properties.mFloorHeight = FixP{atof( std::string{ pos0, pos2}.c_str() )};

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

        while ( ptr <= end ) {

            if ( ptr == end || *ptr == '\n' ) {
                CTileId id = *lineBegin;
                auto props = readPropertiesLine(lineBegin, ptr);
                tokens.clear();
                map[id] = props;
                lineBegin = ptr + 1;
            }

            ++ptr;
        }

        return map;
    }
}

