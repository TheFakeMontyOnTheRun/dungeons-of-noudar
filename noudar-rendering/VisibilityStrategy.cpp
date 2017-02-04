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

	bool VisibilityStrategy::isBlock(const IntMap& occluders, int x, int y) {

		auto tile = occluders[y][x];

		for (auto candidate : {'1', 'Y', 'X', '9', '*', '\\', '/', 'S', 'Z', '|', '%', '<', '>'}) {
			if (candidate == tile) {
				return true;
			}
		}

		return false;
	}

	void VisibilityStrategy::castVisibility(VisMap &visMap, const IntMap &occluders, Knights::Vec2i pos, Knights::EDirection direction) {
		for (auto &line : visMap) {
			std::fill(std::begin(line), std::end(line), EVisibility::kInvisible);
		}

		castVisibility(direction, visMap, occluders, pos, {0, 0});
	}

	Knights::EDirection VisibilityStrategy::getLeftOf(Knights::EDirection d) {
		switch (d) {
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

	Knights::EDirection VisibilityStrategy::getRightOf(Knights::EDirection d) {
		switch (d) {
			case Knights::EDirection::kNorth:
				return Knights::EDirection::kEast;
			case Knights::EDirection::kSouth:
				return Knights::EDirection::kWest;
			case Knights::EDirection::kEast:
				return Knights::EDirection::kSouth;
			case Knights::EDirection::kWest:
				return Knights::EDirection::kNorth;
		}
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

	void VisibilityStrategy::castVisibility(Knights::EDirection from, VisMap &visMap, const IntMap &occluders,
	                                        Knights::Vec2i originalPos, Knights::Vec2i offset) {

		Knights::Vec2i pos = {offset.x + originalPos.x, offset.y + originalPos.y};

		std::array<Knights::Vec2i, Knights::kMapSize + Knights::kMapSize> offsets;
		int stackPos = 0;
		Knights::Vec2i currentPos;
		Knights::Vec2i currentOffset;

		offsets[stackPos] = offset;
		++stackPos;

		while (stackPos > 0) {
			--stackPos;

			currentOffset = offsets[stackPos];
			currentPos = {currentOffset.x + pos.x, currentOffset.y + pos.y};

			if (!isValid(currentPos)) {
				continue;
			}

			if (visMap[currentPos.y][currentPos.x] == EVisibility::kVisible) {
				continue;
			}

			visMap[currentPos.y][currentPos.x] = EVisibility::kVisible;

			if (isBlock(occluders, currentPos.x, currentPos.y)) {
				continue;
			}

			if (currentOffset.y <= 0 && stackPos < offsets.size() - 1 ) {
				auto mapOffset = Knights::mapOffsetForDirerction(from);
				offsets[stackPos] =  Knights::Vec2i{currentOffset.x + mapOffset.x, currentOffset.y + mapOffset.y};
				++stackPos;
			}

			if (0 >= currentOffset.x && stackPos < offsets.size() - 1) {
				auto leftDirection = getLeftOf(from);
				auto leftOffset = Knights::mapOffsetForDirerction(leftDirection);
				offsets[stackPos] = Knights::Vec2i{currentOffset.x + leftOffset.x, currentOffset.y + leftOffset.y};
				++stackPos;
			}

			if (currentOffset.x >= 0 && stackPos < offsets.size() - 1) {
				auto rightDirection = getRightOf(from);
				auto rightOffset = Knights::mapOffsetForDirerction(rightDirection);
				offsets[stackPos] = Knights::Vec2i{currentOffset.x + rightOffset.x, currentOffset.y + rightOffset.y};
				++stackPos;
			}
		}
	}
}
