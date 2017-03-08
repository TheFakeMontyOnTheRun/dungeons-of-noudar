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
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"

#include "NoudarDungeonSnapshot.h"

#include "VisibilityStrategy.h"

namespace odb {

	bool VisibilityStrategy::isValid(Knights::Vec2i pos) {
		return 0 <= pos.x && pos.x < Knights::kMapSize && 0 <= pos.y && pos.y < Knights::kMapSize;
	}

	void VisibilityStrategy::mergeInto(const VisMap &map1, const VisMap &map2, VisMap &result) {
		for ( int y = 0; y < Knights::kMapSize; ++y ) {
			for ( int x = 0; x < Knights::kMapSize; ++x ) {
				if ( map1[ y ][ x ] == EVisibility::kVisible || map2[ y ][ x ] == EVisibility::kVisible ) {
					result[ y ][ x ] = EVisibility::kVisible;
				} else {
					result[ y ][ x ] = EVisibility::kInvisible;
				}
			}
		}
	}

	bool VisibilityStrategy::isBlock(const IntMap& occluders, int x, int y) {

		auto tile = occluders[y][x];

		for (auto candidate : {'1', 'Y', 'X', '\\', '/', 'S', 'Z', '|', '%', '<', '>'}) {
			if (candidate == tile) {
				return true;
			}
		}

		return false;
	}

	void VisibilityStrategy::castVisibility(VisMap &visMap, const IntMap &occluders, Knights::Vec2i pos, Knights::EDirection direction, bool cleanPrevious) {
		if ( cleanPrevious ) {
			for (auto &line : visMap) {
				std::fill(std::begin(line), std::end(line), EVisibility::kInvisible);
			}
		}

		castVisibility(direction, visMap, occluders, pos, {0, 0});
	}

	Knights::EDirection VisibilityStrategy::getOposite(Knights::EDirection d) {
		switch (d) {
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

	bool VisibilityStrategy::isVisibleAt(const VisMap& visMap, Knights::EDirection from, Knights::Vec2i currentPos ) {
		auto converted = transform( from, currentPos );
		return visMap[ converted.y ][ converted.x ] == EVisibility::kVisible;
	}

	void VisibilityStrategy::setIsVisible(VisMap& visMap, Knights::EDirection from, Knights::Vec2i currentPos ) {
		auto converted = transform( from, currentPos );
		visMap[ converted.y ][ converted.x ] = EVisibility::kVisible;
	}

	Knights::Vec2i VisibilityStrategy::transform( Knights::EDirection from, Knights::Vec2i currentPos ) {
		switch( from ) {
			case Knights::EDirection::kNorth:
				return currentPos;
			case Knights::EDirection::kSouth:
				return { Knights::kMapSize - currentPos.x, Knights::kMapSize - currentPos.y };
			case Knights::EDirection::kEast:
				return {currentPos.y, Knights::kMapSize - currentPos.x};
			case Knights::EDirection::kWest:
				return {Knights::kMapSize - currentPos.y, currentPos.x};
		}
	}

	void VisibilityStrategy::castVisibility(Knights::EDirection from, VisMap &visMap, const IntMap &occluders,
	                                        Knights::Vec2i originalPos, Knights::Vec2i offset) {

		Knights::Vec2i pos = {offset.x + originalPos.x, offset.y + originalPos.y};

		std::array<Knights::Vec2i, Knights::kMapSize + Knights::kMapSize> offsets;
		std::array<Knights::Vec2i, Knights::kMapSize + Knights::kMapSize> positions;
		int stackPos = 0;
		Knights::Vec2i currentPos;
		Knights::Vec2i currentOffset;

		offsets[stackPos] = offset;
		positions[stackPos] = pos;
		++stackPos;

		while (stackPos > 0) {
			--stackPos;

			currentOffset = offsets[stackPos];
			currentPos = positions[ stackPos ];

			if (!isValid(currentPos)) {
				continue;
			}

			if ( isVisibleAt( visMap, from, currentPos ) ) {
				continue;
			}

			setIsVisible( visMap, from, currentPos );

			if (isBlock(occluders, currentPos.x, currentPos.y)) {
				continue;
			}

			auto leftDirection = leftOf(from);
			auto rightDirection = rightOf(from);
			auto rightOffset = Knights::mapOffsetForDirection(rightDirection);
			auto leftOffset = Knights::mapOffsetForDirection(leftDirection);


            //The -1 is due to the fact I will add a new element.
			if ( ( originalPos.x - currentPos.x ) >= 0 && stackPos < positions.size() - 1) {
				positions[stackPos] =  Knights::Vec2i{currentPos.x + leftOffset.x, currentPos.y + leftOffset.y};
				offsets[stackPos] =  Knights::Vec2i{currentOffset.x + leftOffset.x, currentOffset.y + leftOffset.y};
				++stackPos;
			}

			if ( ( originalPos.x - currentPos.x ) <= 0 && stackPos < positions.size() - 1) {
				positions[stackPos] =  Knights::Vec2i{currentPos.x + rightOffset.x, currentPos.y + rightOffset.y};
				offsets[stackPos] =  Knights::Vec2i{currentOffset.x + rightOffset.x, currentOffset.y + rightOffset.y};
				++stackPos;
			}

			if (currentOffset.y <= 0 && stackPos < positions.size() - 1) {
				auto mapOffset = Knights::mapOffsetForDirection(from);
				offsets[stackPos] =  Knights::Vec2i{currentOffset.x + mapOffset.x, currentOffset.y + mapOffset.y};
				positions[stackPos] =  Knights::Vec2i{currentPos.x + mapOffset.x, currentPos.y + mapOffset.y};
				++stackPos;
			}

		}
	}
}
