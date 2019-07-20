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

#include <fstream>

#include <cstdlib>
#include <vector>
#include <array>

using std::vector;
using std::array;


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

#include "SoundClip.h"
#include "SoundListener.h"
#include "SoundEmitter.h"

#include "NoudarDungeonSnapshot.h"
#include "GameNativeAPI.h"
#include "Logger.h"
#include "VisibilityStrategy.h"

#include "LightningStrategy.h"

namespace odb {

    const bool kWillAttemptToMergeVisibilityToFillPotetialHoles = true;

	Knights::EDirection previousDirection = Knights::EDirection::kNorth;
	Knights::Vec2i previousPosition = {0,0};
	VisMap previous;

  NoudarGLES2Bridge::NoudarGLES2Bridge() {
  }

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


                auto element = map.getElementAt( {x, y} );

                if ( element != '0' ) {
                    snapshot.map[ y ][ x ] = element;
                }

                if ( map.getItemAt( {x,y})) {
                    snapshot.mItemMap[ y ][ x ] = map.getItemViewAt( {x,y} );
                } else {
                    snapshot.mItemMap[ y ][ x ] = 0;
                }

	            auto actor = map.getActorAt({ x, y } );

	            if ( actor != nullptr && actor->isAlive()) {

                    snapshot.ids[ y ][ x ] = actor->getId();

                    bool alternate = (actor->getMoves() % 2) == 0;

                    switch ( actor->getView()) {
                        case 'T':
                            snapshot.snapshot[ y ][ x ] = EActorsSnapshotElement::kRope;
                            continue;
                        case 'C':
                            snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kCocoonStanding0 : EActorsSnapshotElement::kCocoonStanding1;
                            continue;
                        case '@':
                            if ( actor->getStance() == Knights::EStance::kAttacking ) {
                                snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kMonkAttacking0 : EActorsSnapshotElement::kMonkAttacking1;
                            } else {
                                snapshot.snapshot[ y ][ x ] =  ( alternate ) ? EActorsSnapshotElement::kMonkStanding0 : EActorsSnapshotElement::kMonkStanding1;
                            }
                            continue;
                        case '$':
                            if ( actor->getStance() == Knights::EStance::kAttacking ) {
                                snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kFallenAttacking0 : EActorsSnapshotElement::kFallenAttacking1;
                            } else {
                                snapshot.snapshot[ y ][ x ] =  ( alternate ) ? EActorsSnapshotElement::kFallenStanding0 : EActorsSnapshotElement::kFallenStanding1;
                            }
                            continue;
                        case 'w':
                            if ( actor->getStance() == Knights::EStance::kAttacking ) {
                                snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kEvilSpiritAttacking0 : EActorsSnapshotElement::kEvilSpiritAttacking1;
                            } else {
                                snapshot.snapshot[ y ][ x ] =  ( alternate ) ? EActorsSnapshotElement::kEvilSpiritStanding0 : EActorsSnapshotElement::kEvilSpiritStanding1;\
                            }
                            continue;
                        case 'J':
                            if ( actor->getStance() == Knights::EStance::kAttacking ) {
                                snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kWarthogAttacking0 : EActorsSnapshotElement::kWarthogAttacking1;
                            } else {
                                snapshot.snapshot[ y ][ x ] =  ( alternate ) ? EActorsSnapshotElement::kWarthogStanding0: EActorsSnapshotElement::kWarthogStanding1;
                            }
                            continue;
                        case 'd':
                            if ( actor->getStance() == Knights::EStance::kAttacking ) {
                                snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kWeakenedDemonAttacking0 : EActorsSnapshotElement::kWeakenedDemonAttacking1;
                            } else {
                                snapshot.snapshot[ y ][ x ] =  ( alternate ) ? EActorsSnapshotElement::kWeakenedDemonStanding0 : EActorsSnapshotElement::kWeakenedDemonStanding1;
                            }
                            continue;
                        case 'D':
                            if ( actor->getStance() == Knights::EStance::kAttacking ) {
                                snapshot.snapshot[ y ][ x ] = ( alternate ) ? EActorsSnapshotElement::kStrongDemonAttacking0 : EActorsSnapshotElement::kStrongDemonAttacking1;
                            } else {
                                snapshot.snapshot[ y ][ x ] =  ( alternate ) ? EActorsSnapshotElement::kStrongDemonStanding0 : EActorsSnapshotElement::kStrongDemonStanding1;
                            }
                            continue;

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

        if ( mPerformVisibilityCheck ) {
            if ( kWillAttemptToMergeVisibilityToFillPotetialHoles  ) {
                VisibilityStrategy::castVisibility( currentVisMap, snapshot.map,  cameraPosition, current->getDirection(), true );
                VisibilityStrategy::castVisibility( previous, snapshot.map,  previousPosition, previousDirection, true );
                VisibilityStrategy::mergeInto( currentVisMap, previous, snapshot.mVisibilityMap);
            } else {
                VisibilityStrategy::castVisibility( snapshot.mVisibilityMap, snapshot.map,  cameraPosition, current->getDirection(), true );
            }
        } else {
            VisibilityStrategy::makeAllVisible( snapshot.mVisibilityMap );
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

    char NoudarGLES2Bridge::peekInput() {
      return mNextCmd;
    }

    char NoudarGLES2Bridge::getInput() {

	    char tmp = mNextCmd;
	    mNextCmd = '.';
        return tmp;
    }

	void NoudarGLES2Bridge::setNextCommand(char cmd) {
		mNextCmd = cmd;
	}

    void NoudarGLES2Bridge::setVisibilityChecks(bool visibilityCheck) {
        mPerformVisibilityCheck = visibilityCheck;
    }
}
