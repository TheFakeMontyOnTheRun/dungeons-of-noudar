//
// Created by monty on 30/07/16.
//

#include <map>
#include <memory>
#include <functional>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;

#include <sg14/fixed_point>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using sg14::fixed_point;
using eastl::vector;
using eastl::array;

#include "NativeBitmap.h"
#include "ETextures.h"
#include "IFileLoaderDelegate.h"
#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "VisibilityStrategy.h"

namespace odb {

    const bool kNarrowByDistance = true;

	bool VisibilityStrategy::isValid(const Knights::Vec2i& pos) {
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

	bool VisibilityStrategy::isBlock(const IntMap& occluders, Knights::EDirection direction, const Knights::Vec2i& currentPos) {

        auto transformed = transform( direction, currentPos );

		auto tile = occluders[ transformed.y][transformed.x];

		for (auto candidate : {'1', 'I','Y', 'X', 'R', '\\', '/', 'S', 'Z', '|', '%', '<', '>'}) {
			if (candidate == tile) {
				return true;
			}
		}

		return false;
	}

	void VisibilityStrategy::castVisibility(VisMap &visMap, const IntMap &occluders, const Knights::Vec2i& pos, Knights::EDirection direction, bool cleanPrevious, DistanceDistribution& distances) {
		if ( cleanPrevious ) {
			for (auto &line : visMap) {
				std::fill(std::begin(line), std::end(line), EVisibility::kInvisible);
			}

			for ( auto& distanceLine : distances ) {
				distanceLine.clear();
				distanceLine.reserve(Knights::kMapSize + Knights::kMapSize);
			}
		}

		castVisibility(direction, visMap, occluders, transform( direction, pos ), {0, 0}, distances);
	}

	bool VisibilityStrategy::isVisibleAt(const VisMap& visMap, Knights::EDirection from, const Knights::Vec2i& currentPos ) {
		auto converted = transform( from, currentPos );
		return visMap[ converted.y ][ converted.x ] == EVisibility::kVisible;
	}

	void VisibilityStrategy::setIsVisible(VisMap& visMap, Knights::EDirection from, const Knights::Vec2i& currentPos ) {
		auto converted = transform( from, currentPos );
		visMap[ converted.y ][ converted.x ] = EVisibility::kVisible;
	}

	Knights::Vec2i VisibilityStrategy::transform( Knights::EDirection from, const Knights::Vec2i& currentPos ) {

		switch( from ) {
			case Knights::EDirection::kNorth:
				return currentPos;
			case Knights::EDirection::kSouth:
				return { Knights::kMapSize - currentPos.x - 1, Knights::kMapSize - currentPos.y - 1};
			case Knights::EDirection::kEast:
				return { Knights::kMapSize - currentPos.y - 1, Knights::kMapSize - currentPos.x - 1};
			case Knights::EDirection::kWest:
				return { currentPos.y, currentPos.x};
		}
	}

	void VisibilityStrategy::castVisibility(Knights::EDirection from, VisMap &visMap, const IntMap &occluders,
	                                        const Knights::Vec2i& originalPos, const Knights::Vec2i& offset, DistanceDistribution& distances) {

		array<Knights::Vec2i, Knights::kMapSize + Knights::kMapSize> positions;
		int stackPos = 0;
		Knights::Vec2i currentPos;

		positions[stackPos] = originalPos;
		++stackPos;

		while (stackPos > 0) {
			--stackPos;

			currentPos = positions[ stackPos ];

			auto transformed = transform( from, currentPos);

			if (!isValid(transformed)) {
				continue;
			}

			if ( isVisibleAt( visMap, from, currentPos ) ) {
				continue;
			}

			setIsVisible( visMap, from, currentPos );

            int manhattanDistance = std::abs( currentPos.y - originalPos.y ) + std::abs( currentPos.x - originalPos.x );
            distances[ manhattanDistance ].push_back( transformed );

            if (isBlock(occluders, from, currentPos )) {
				continue;
			}

			auto leftDirection = Knights::EDirection::kWest;
			auto rightDirection = Knights::EDirection::kEast;
			auto rightOffset = Knights::mapOffsetForDirection(rightDirection);
			auto leftOffset = Knights::mapOffsetForDirection(leftDirection);

            //The -1 is due to the fact I will add a new element.

            int distance = ( currentPos.y - originalPos.y );
            int narrowing = kNarrowByDistance ? distance : 1;

			if ( ( !kNarrowByDistance || ( currentPos.x - originalPos.x ) >= -std::abs(narrowing) )&& ( currentPos.x - originalPos.x ) <= 0 && stackPos < positions.size() - 1) {
				positions[stackPos] =  Knights::Vec2i{currentPos.x + leftOffset.x, currentPos.y + leftOffset.y};
				++stackPos;
			}

			if ( ( !kNarrowByDistance || ( currentPos.x - originalPos.x ) <= std::abs(narrowing) ) && ( currentPos.x - originalPos.x ) >= 0 && stackPos < positions.size() - 1) {
				positions[stackPos] =  Knights::Vec2i{currentPos.x + rightOffset.x, currentPos.y + rightOffset.y};
				++stackPos;
			}

            if (  distance <= 0 && stackPos < positions.size() - 1) {
                auto mapOffset = Knights::mapOffsetForDirection(Knights::EDirection::kNorth);
                positions[stackPos] =  Knights::Vec2i{currentPos.x + mapOffset.x, currentPos.y + mapOffset.y};
                ++stackPos;
            }


		}
	}

    void VisibilityStrategy::makeAllVisible(VisMap& map) {
		for ( int y = 0; y < Knights::kMapSize; ++y ) {
			for ( int x = 0; x < Knights::kMapSize; ++x ) {
				map[ y ][ x ] = EVisibility::kVisible;
			}
		}
    }
}
