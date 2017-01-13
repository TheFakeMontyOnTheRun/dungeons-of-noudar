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
#include "CTeam.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"

#include "IFileLoaderDelegate.h"
#include "CGame.h"
#include "NoudarGLES2Bridge.h"


#include "AudioSink.h"
#include "SoundClip.h"
#include "SoundUtils.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "NoudarDungeonSnapshot.h"

#include "GameNativeAPI.h"

#include "Logger.h"

namespace odb {
    void NoudarGLES2Bridge::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {

        odb::NoudarDungeonSnapshot snapshot;

        for ( int y = 0; y < Knights::kMapSize; ++y ) {
            for ( int x = 0; x < Knights::kMapSize; ++x ) {

                snapshot.map[ y ][ x ] = '.';
                snapshot.snapshot[ y ][ x ] = '.';
                snapshot.ids[ y ][ x ] = 0;
                snapshot.splat[ y ][ x ] = -1;

	            auto element = map.getElementAt( x, y );

	            if ( element != '0' ) {
                    snapshot.map[ y ][ x ] = element;
	            }

	            auto actor = map.getActorAt({ x, y } );

	            if ( actor != nullptr) {

                    snapshot.ids[ y ][ x ] = actor->getId();

                    if ( actor->getTeam() != current->getTeam() ) {

                        bool alternate = (actor->getMoves() % 2) == 0;

                        if ( actor->getStance() == Knights::EStance::kAttacking ) {
                            snapshot.snapshot[ y ][ x ] = ( alternate ) ? 'J' : 'j';
                        } else {
                            snapshot.snapshot[ y ][ x ] =  ( alternate ) ? 'K' : 'k';
                        }


                    } else {
                        snapshot.snapshot[ y ][ x ] = (actor == current)? '^' : '?';
                    }
                }
            }
        }

        auto cameraPosition = current->getPosition();
        setCameraPosition(cameraPosition.x, cameraPosition.y);
        setSnapshot( snapshot );
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
