//
// Created by monty on 30/07/16.
//

#include "glm/glm.hpp"

#include <memory>
#include <iostream>
#include <array>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <map>
#include <vector>
#include "NativeBitmap.h"
#include "Texture.h"
#include "Material.h"
#include "Trig.h"
#include "TrigBatch.h"
#include "MeshObject.h"
#include "MaterialList.h"
#include "Scene.h"
#include "VBORenderingJob.h"

#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"

#include "NoudarDungeonSnapshot.h"

#include "VisibilityStrategy.h"

namespace odb {

	bool VisibilityStrategy::isValid(Knights::Vec2i pos) {
		return 0 <= pos.x && pos.x < Knights::kMapSize && 0 <= pos.y && pos.y < Knights::kMapSize;
	}

	bool VisibilityStrategy::isBlock(IntMap occluders, int x, int y) {

		auto tile = occluders[y][x];

		for (auto candidate : { '1', 'Y', 'X', '9', '*', '\\', '/', 'S', 'Z', '|', '%', '<', '>' }) {
			if (candidate == tile) {
				return true;
			}
		}

		return false;
	}

	void VisibilityStrategy::castVisibility(IntMap &visMap, IntMap& occluders, Knights::Vec2i pos) {
		for ( auto& line : visMap ) {
			std::fill( std::begin( line ), std::end( line ), 0 );
		}

		castVisibility( Knights::EDirection::kNorth, visMap, occluders, pos );
	}

	Knights::EDirection VisibilityStrategy::getLeftOf( Knights::EDirection d ) {
		switch( d ) {
			case Knights::EDirection::kNorth:
				return Knights::EDirection::kWest;
			case Knights::EDirection::kSouth:
				return Knights::EDirection::kEast;
			case Knights::EDirection::kEast:
				return Knights::EDirection::kNorth;
			case Knights::EDirection::kWest:
				return Knights::EDirection::kSouth;
		}
	}

	Knights::EDirection VisibilityStrategy::getRightOf( Knights::EDirection d ) {
		switch( d ) {
			case Knights::EDirection::kNorth:
				return Knights::EDirection::kWest;
			case Knights::EDirection::kSouth:
				return Knights::EDirection::kEast;
			case Knights::EDirection::kEast:
				return Knights::EDirection::kNorth;
			case Knights::EDirection::kWest:
				return Knights::EDirection::kSouth;
		}
	}

	Knights::EDirection VisibilityStrategy::getOposite( Knights::EDirection d ) {
		switch( d ) {
			case Knights::EDirection::kNorth:
				return Knights::EDirection::kSouth;
			case Knights::EDirection::kSouth:
				return Knights::EDirection::kNorth;
			case Knights::EDirection::kEast:
				return Knights::EDirection::kWest;
			case Knights::EDirection::kWest:
				return Knights::EDirection::kEast;
		}
	}

	Knights::Vec2i VisibilityStrategy::getOffsetFor( Knights::EDirection d, Knights::Vec2i accumulated ) {
		switch( d ) {
			case Knights::EDirection::kNorth:
				return Knights::Vec2i{ accumulated.x , accumulated.y -1 };
			case Knights::EDirection::kSouth:
				return Knights::Vec2i{ accumulated.x , accumulated.y + 1};
			case Knights::EDirection::kEast:
				return Knights::Vec2i{ accumulated.x + 1, accumulated.y };
			case Knights::EDirection::kWest:
				return Knights::Vec2i{ accumulated.x - 1, accumulated.y };
		}
	}

	void VisibilityStrategy::castVisibility(Knights::EDirection from, IntMap &visMap, IntMap& occluders, Knights::Vec2i pos) {

		std::array< Knights::Vec2i, Knights::kMapSize * Knights::kMapSize > offsets;
		std::array< Knights::EDirection, Knights::kMapSize * Knights::kMapSize > origins;
		int stackPos = 0;
		Knights::Vec2i currentPos;
		Knights::Vec2i currentOffset;
		Knights::EDirection currentFrom;

		offsets[ stackPos ] = {0, 0};
		origins[ stackPos ] = from;
		++stackPos;

		while( stackPos > 0 ) {
			--stackPos;
			currentFrom = origins[ stackPos ];
			currentOffset = offsets[ stackPos ];
			currentPos = { currentOffset.x + pos.x, currentOffset.y + pos.y };

			if (!isValid(pos)) {
				return;
			}

			if ( isBlock( occluders, currentPos.x, currentPos.y ) ) {
				return;
			}

			visMap[ currentPos.y ][ currentPos.x ] = 1;

			if ( currentFrom != Knights::EDirection::kNorth && currentOffset.y > 0) {
				offsets[ stackPos ] = Knights::Vec2i{ currentOffset.x, currentOffset.y - 1};
				origins[ stackPos ] = Knights::EDirection::kNorth;
				++stackPos;
			}

//			if ( currentFrom != Knights::EDirection::E &&  0 <= currentOffset.x ) {
//				offsets[ stackPos ] = Knights::Vec2i{ currentOffset.x - 1, currentOffset.y};
//				origins[ stackPos ] = Knights::EDirection::W;
//				++stackPos;
//			}
//
//
//			if ( currentFrom != Knights::EDirection::W && currentOffset.x <= 0) {
//				offsets[ stackPos ] = Knights::Vec2i{ currentOffset.x + 1, currentOffset.y};
//				origins[ stackPos ] = Knights::EDirection::E;
//				++stackPos;
//			}

		}
	}
}
