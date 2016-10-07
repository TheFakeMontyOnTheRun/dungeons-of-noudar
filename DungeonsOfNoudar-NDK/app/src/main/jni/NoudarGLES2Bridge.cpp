//
// Created by monty on 06/10/16.
//



#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <utility>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <array>
#include <fstream>
#include <vector>

#include "Vec2i.h"
#include "gles2-renderer/NativeBitmap.h"
#include "IMapElement.h"
#include "CActor.h"
#include "CMap.h"
#include "IRenderer.h"
#include "CFalconKnight.h"
#include "CBullKnight.h"
#include "CTurtleKnight.h"
#include "CGame.h"
#include "NoudarGLES2Bridge.h"
#include "GameNativeAPI.h"

#include "gles2-renderer/Logger.h"

namespace odb {
    void NoudarGLES2Bridge::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {

        std::array<int, 400> level;
        std::array<int, 400> actors;
        std::array<int, 400> splats;
        std::array<int, 400> ids;

        for (auto &item : level) {
            item = '.';
        }

        for (auto &item : actors) {
            item = 0;
        }

        for (auto &item : splats) {
            item = -1;
        }

        for (auto &item: ids) {
            item = 0;
        }

        int position =  0;

        for ( int y = 0; y < 20; ++y ) {
            for ( int x = 0; x < 20; ++x ) {
                position = ( y * 20 ) + x;

	            auto actor = map.getActorAt({ x, y } );

	            if ( actor != nullptr && actor != current ) {
		            ids[ position ] = actor->getId();
		            actors[ position ] = '@';
	            } else {
		            auto element = map.getElementAt( x, y );

		            if ( element != '0' ) {
			            level[ position ] = element;
		            }
	            }
            }
        }

	    position = ( current->getPosition().y * 20 ) + current->getPosition().x;
	    ids[ position ] = current->getId();
	    actors[ position ] = '^';
	    level[ position ] = '.';


	    updateLevelSnapshot(level.data(), actors.data(), splats.data());
        updateCharacterMovements(ids.data());

        auto cameraPosition = current->getPosition();
        setCameraPosition(cameraPosition.x, cameraPosition.y);
    }

    char NoudarGLES2Bridge::getInput() {

	    char tmp = mNextCmd;
	    mNextCmd = '.';
        return tmp;
    }

	void NoudarGLES2Bridge::setNextCommand(char cmd) {
		mNextCmd = cmd;
	}
}
