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
#include "VisibilityStrategy.h"

namespace odb {
    void NoudarGLES2Bridge::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {

        odb::NoudarDungeonSnapshot snapshot;

        for ( int y = 0; y < Knights::kMapSize; ++y ) {
            for ( int x = 0; x < Knights::kMapSize; ++x ) {

                snapshot.map[ y ][ x ] = '.';
                snapshot.snapshot[ y ][ x ] = EActorsSnapshotElement::kNothing;
                snapshot.ids[ y ][ x ] = 0;
                snapshot.splat[ y ][ x ] = -1;

	            auto element = map.getElementAt( x, y );

	            if ( element != '0' ) {
                    snapshot.map[ y ][ x ] = element;
	            }

	            auto actor = map.getActorAt({ x, y } );

	            if ( actor != nullptr && actor->isAlive()) {

                    snapshot.ids[ y ][ x ] = actor->getId();
                    bool alternate = (actor->getMoves() % 2) == 0;

                    if ( actor->getTeam() != current->getTeam() ) {
                        if ( actor->getStance() == Knights::EStance::kAttacking ) {
                            snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kDemonAttacking0 : EActorsSnapshotElement::kDemonAttacking1;
                        } else {
                            snapshot.snapshot[ y ][ x ] =  ( alternate ) ? EActorsSnapshotElement::kDemonStanding0 : EActorsSnapshotElement::kDemonStanding1;
                        }
                    } else {
                        if ( actor->getStance() == Knights::EStance::kAttacking ) {
                            snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kHeroAttacking0 : EActorsSnapshotElement::kHeroAttacking1;
                        } else {
                            snapshot.snapshot[ y ][ x ] =  ( alternate ) ? EActorsSnapshotElement::kHeroStanding0: EActorsSnapshotElement::kHeroStanding1;
                        }
                    }
                }
            }
        }

        auto cameraPosition = current->getPosition();
        setCameraPosition(cameraPosition.x, cameraPosition.y);

	    odb::VisibilityStrategy::castVisibility( snapshot.mVisibilityMap, snapshot.map, cameraPosition, current->getDirection());
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
