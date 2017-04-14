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
#include "CItem.h"
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
#include "LightningStrategy.h"

namespace odb {

    const bool kWillAttemptToMergeVisibilityToFillPotetialHoles =
#ifdef OSMESA
            false;
#else
            true;
#endif

	Knights::EDirection previousDirection = Knights::EDirection::kNorth;
	Knights::Vec2i previousPosition = {0,0};
	VisMap previous;

    void NoudarGLES2Bridge::drawMap(Knights::CMap &map, std::shared_ptr<Knights::CActor> current) {

        odb::NoudarDungeonSnapshot snapshot;

        for ( int y = 0; y < Knights::kMapSize; ++y ) {
            for ( int x = 0; x < Knights::kMapSize; ++x ) {

                snapshot.mLightMap[ y ][ x ] = 192;
                snapshot.map[ y ][ x ] = '.';
                snapshot.snapshot[ y ][ x ] = EActorsSnapshotElement::kNothing;
                snapshot.ids[ y ][ x ] = 0;
                snapshot.splat[ y ][ x ] = -1;
                snapshot.mVisibilityMap[ y ][ x ] = EVisibility::kVisible;

	            auto element = map.getMapAt( {x, y} );

	            if ( element != '0' ) {
                    snapshot.map[ y ][ x ] = element;
	            }

                if ( map.getItemAt( {x,y})) {
                    snapshot.mItemMap[ y ][ x ] = map.getElementAt( {x,y} );
                } else {
                    snapshot.mItemMap[ y ][ x ] = 0;
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

        for ( int y = 0; y < Knights::kMapSize; ++y ) {
            for (int x = 0; x < Knights::kMapSize; ++x) {
                auto actor = map.getActorAt({ x, y } );


                if ( actor != nullptr && actor->isAlive()  ) {
                    LightningStrategy::castPointLight(snapshot.mLightMap, 64, snapshot.map, x, y);
                }
            }
        }


        auto cameraPosition = current->getPosition();
	    VisMap currentVisMap;

        if ( kWillAttemptToMergeVisibilityToFillPotetialHoles  ) {
            VisibilityStrategy::castVisibility( currentVisMap, snapshot.map,  cameraPosition, current->getDirection(), true );
            VisibilityStrategy::castVisibility( previous, snapshot.map,  previousPosition, previousDirection, true );
            VisibilityStrategy::mergeInto( currentVisMap, previous, snapshot.mVisibilityMap);
        } else {
            VisibilityStrategy::castVisibility( snapshot.mVisibilityMap, snapshot.map,  cameraPosition, current->getDirection(), true );
        }

        auto item = current->getSelectedItem();

        if ( item != nullptr ) {
            snapshot.mCurrentItem = item->to_string();
        } else {
            snapshot.mCurrentItem = "";
        }

        snapshot.mHP = current->getHP();

        setSnapshot( snapshot );

	    previousPosition = cameraPosition;
	    previousDirection = current->getDirection();
	    previous = snapshot.mVisibilityMap;
    }

    void NoudarGLES2Bridge::reset() {
        previousPosition = {0, 0};
        previousDirection = Knights::EDirection::kNorth;
        previous = odb::VisMap();
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
