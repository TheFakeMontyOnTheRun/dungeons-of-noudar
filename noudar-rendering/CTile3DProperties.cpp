//
// Created by monty on 14/01/17.
//
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef __APPLE__
#if TARGET_IOS
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#endif
#else

#endif

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <unordered_set>
#include <map>
#include <array>

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"

#include "NativeBitmap.h"
#include "Logger.h"
#include "VBORenderingJob.h"
#include "NoudarDungeonSnapshot.h"
#include "ETextures.h"
#include "VBORegister.h"
#include "CTile3DProperties.h"

namespace odb {

    double atod( const char *s ) {
        std::stringstream ss;

        ss << s;

        double toReturn;

        ss >> toReturn;

        return toReturn;
    }

    CTile3DProperties readPropertiesLine(std::vector<std::string>::iterator& pos) {

        CTile3DProperties properties;

        pos = std::next(pos);
        properties.mNeedsAlphaTest = std::atoi(pos->c_str()) == 1;
        pos = std::next(pos);
        properties.mCeilingTexture = *pos;
        pos = std::next(pos);
        properties.mFloorTexture = *pos;
        pos = std::next(pos);
        properties.mMainWallTexture = *pos;
        pos = std::next(pos);
        properties.mVBOToRender = *pos;
        pos = std::next(pos);
        properties.mCeilingRepeatedWallTexture = *pos;
        pos = std::next(pos);
        properties.mFloorRepeatedWallTexture = *pos;
        pos = std::next(pos);
        properties.mCeilingRepetitions = std::atoi( pos->c_str() );
        pos = std::next(pos);
        properties.mFloorRepetitions = std::atoi( pos->c_str() );
        pos = std::next(pos);
        properties.mCeilingHeight = atof( pos->c_str() );
        pos = std::next(pos);
        properties.mFloorHeight = atof( pos->c_str() );

        return properties;
    }


    CTilePropertyMap CTile3DProperties::parsePropertyList(std::string propertyFile) {

        odb::CTilePropertyMap map;

        std::stringstream ss;
        ss << propertyFile;

        std::vector<std::string> tokens{std::istream_iterator<std::string>(ss),
                                        std::istream_iterator<std::string>{}};

        auto fileBegin = std::begin(tokens);
        auto fileEnd = std::end(tokens);
        auto pos = fileBegin;

        while (pos != fileEnd) {
            CTileId id = pos->c_str()[0];
            auto props = readPropertiesLine(pos);
	        map[id] = props;
            pos = std::next(pos);
        }

        return map;
    }
}

