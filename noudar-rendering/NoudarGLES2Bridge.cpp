//
// Created by monty on 06/10/16.
//



#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <utility>
#include <functional>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <array>
#include <fstream>
#include <vector>
#include <cstdlib>

#include "Vec2i.h"
#include "NativeBitmap.h"
#include "IMapElement.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "CKnight.h"

#include "IFileLoaderDelegate.h"
#include "CGame.h"
#include "NoudarGLES2Bridge.h"


#include "AudioSink.h"
#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"


#include "GameNativeAPI.h"

#include "Logger.h"

namespace odb {
    void NoudarGLES2Bridge::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {

        std::array<int, Knights::kMapSize * Knights::kMapSize> level;
        std::array<int, Knights::kMapSize * Knights::kMapSize> actors;
        std::array<int, Knights::kMapSize * Knights::kMapSize> splats;
        std::array<int, Knights::kMapSize * Knights::kMapSize> ids;

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

        for ( int y = 0; y < Knights::kMapSize; ++y ) {
            for ( int x = 0; x < Knights::kMapSize; ++x ) {
                position = ( y * Knights::kMapSize) + x;
	            auto element = map.getElementAt( x, y );

	            if ( element != '0' ) {
		            level[ position ] = element;
	            }

	            auto actor = map.getActorAt({ x, y } );

	            if ( actor != nullptr) {
                    ids[ position ] = actor->getId();
                    if ( actor->getTeam() != current->getTeam() ) {
                        actors[ position ] = '@';
                    } else {
                        actors[ position ] = (actor == current)? '^' : '?';
                    }
                }
            }
        }

	    position = ( current->getPosition().y * Knights::kMapSize ) + current->getPosition().x;
	    ids[ position ] = current->getId();
	    actors[ position ] = '^';

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
