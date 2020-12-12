//
// Created by monty on 30/07/16.
//

#include <map>
#include <memory>
#include <functional>
#include <vector>
#include <array>

using std::vector;
using std::array;

#include <sg14/fixed_point>

using sg14::fixed_point;

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
#include "MapWithCharKey.h"
#include "CMap.h"
#include "VisibilityStrategy.h"

namespace odb {

    const bool kNarrowByDistance = true;
	const bool kConservativeOccluders = true;
	const int kUseLimitedDrawingDistance = false;
	const int kDrawingDistance = 20;

	bool VisibilityStrategy::isValid(const Knights::Vec2i& pos) {
		return 0 <= pos.x && pos.x < Knights::kMapSize && 0 <= pos.y && pos.y < Knights::kMapSize;
	}

	bool VisibilityStrategy::isBlock(const IntMap& occluders, const Knights::Vec2i& transformed) {

		auto tile = occluders[ transformed.y][transformed.x];

		std::string occluderString = "";

		if (kConservativeOccluders ) {
			occluderString = "1IYXPR";
		} else {
			occluderString = "1IYXPR\\/SZ|%<>";
		}

		for (const auto& candidate : occluderString ) {
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
		}

		for ( auto& distanceLine : distances ) {
			distanceLine.clear();
		}

		castVisibility(direction, visMap, occluders, transform( direction, pos ), distances);
	}

	bool VisibilityStrategy::isVisibleAt(const VisMap& visMap, const Knights::Vec2i& transformed ) {
		return visMap[ transformed.y ][ transformed.x ] == EVisibility::kVisible;
	}

	void VisibilityStrategy::setIsVisible(VisMap& visMap, const Knights::Vec2i& transformed ) {
		visMap[ transformed.y ][ transformed.x ] = EVisibility::kVisible;
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
	                                        const Knights::Vec2i& originalPos, DistanceDistribution& distances) {

		array<Knights::Vec2i, Knights::kMapSize + Knights::kMapSize> positions;
		Knights::Vec2i currentPos;

		//The -1 is due to the fact I will add a new element.
		auto stackEnd = std::end( positions ) - 1;
		auto stackHead = std::begin(positions);
		auto stackRoot = stackHead;

		auto rightOffset = Knights::mapOffsetForDirection(Knights::EDirection::kEast);
		auto leftOffset = Knights::mapOffsetForDirection(Knights::EDirection::kWest);
		auto northOffset = Knights::mapOffsetForDirection(Knights::EDirection::kNorth);

		*stackHead = originalPos;
        ++stackHead;

		while (stackHead != stackRoot ) {
            --stackHead;

            currentPos = *stackHead;

			auto transformed = transform( from, currentPos);

			if (!isValid(transformed)) {
				continue;
			}

			if ( isVisibleAt( visMap, transformed ) ) {
				continue;
			}

			setIsVisible( visMap, transformed );

			int verticalDistance = ( currentPos.y - originalPos.y );

			if (kUseLimitedDrawingDistance ) {
				if (std::abs(verticalDistance) > kDrawingDistance) {
					continue;
				}
			}

            int manhattanDistance = std::abs( verticalDistance ) + std::abs( currentPos.x - originalPos.x );
            distances[ manhattanDistance ].push_back( transformed );

            if (isBlock(occluders, transformed )) {
				continue;
			}

            int narrowing = kNarrowByDistance ? std::abs(verticalDistance) : 1;

			if ( ( !kNarrowByDistance || ( currentPos.x - originalPos.x ) >= -narrowing )&& ( currentPos.x - originalPos.x ) <= 0 && (stackHead != stackEnd ) ) {
				*stackHead++ =  Knights::Vec2i{currentPos.x + leftOffset.x, currentPos.y + leftOffset.y};
			}

			if ( ( !kNarrowByDistance || ( currentPos.x - originalPos.x ) <= narrowing ) && ( currentPos.x - originalPos.x ) >= 0 && (stackHead != stackEnd ) ) {
				*stackHead++ =  Knights::Vec2i{currentPos.x + rightOffset.x, currentPos.y + rightOffset.y};
			}

            if (  verticalDistance <= 0 && (stackHead != stackEnd) ) {
                *stackHead++ =  Knights::Vec2i{currentPos.x + northOffset.x, currentPos.y + northOffset.y};
            }

		}
	}
}
