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

#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <unordered_set>
#include <map>
#include <vector>
#include <array>

using std::vector;
using std::array;

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

    CTile3DProperties readPropertiesLine(vector<std::string>::iterator &pos) {

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
        properties.mVBOToRender = *pos;
        pos = std::next(pos);
        properties.mCeilingRepeatedWallTexture = *pos;
        pos = std::next(pos);
        properties.mFloorRepeatedWallTexture = *pos;
        pos = std::next(pos);
        properties.mCeilingRepetitions = std::atoi(pos->c_str());
        pos = std::next(pos);
        properties.mFloorRepetitions = std::atoi(pos->c_str());
        pos = std::next(pos);
        properties.mCeilingHeight = atof(pos->c_str());
        pos = std::next(pos);
        properties.mFloorHeight = atof(pos->c_str());

        return properties;
    }


    CTilePropertyMap CTile3DProperties::parsePropertyList(std::string propertyFile) {

        odb::CTilePropertyMap map;

        std::stringstream ss;
        ss << propertyFile;

        vector<std::string> tokens;
        std::string tmp;
        while (ss.good()) {
            ss >> tmp;
            tokens.push_back(tmp);
        }

        auto fileBegin = std::begin(tokens);
        auto fileEnd = std::end(tokens);
        auto pos = fileBegin;

        int line = 0;

        while (pos != fileEnd) {
            line++;
            CTileId id = pos->c_str()[0];
            auto props = readPropertiesLine(pos);
            map[id] = props;
            pos = std::next(pos);
        }

        return map;
    }
}

