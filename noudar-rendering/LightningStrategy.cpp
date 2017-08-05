//
// Created by monty on 30/07/16.
//

#include "glm/glm.hpp"

#include <memory>
#include <functional>
#include <iostream>
#include <array>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <map>
#include <vector>
#include "NativeBitmap.h"
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

#include "LightningStrategy.h"

namespace odb {

	bool LightningStrategy::isValid(Knights::Vec2i pos) {
		return 0 <= pos.x && pos.x < Knights::kMapSize && 0 <= pos.y && pos.y < Knights::kMapSize;
	}

	void LightningStrategy::castPointLight(LightMap &lightMap, int emission, IntMap occluders,
	                                       int x, int y) {
		castLight(Direction::TOP, lightMap, emission, occluders, Knights::Vec2i{x, y});
	}

	bool isBlock(IntMap occluders, int x, int y) {

		auto tile = occluders[y][x];

		for (auto candidate : { '1', 'Y', 'X', '9', '*'}) {
			if (candidate == tile) {
				return true;
			}
		}

		return false;
	}


	void LightningStrategy::castLight(Direction from, LightMap &lightMap, int emission,
	                                  IntMap occluders, Knights::Vec2i originalPos) {

		std::array<Knights::Vec2i, Knights::kMapSize + Knights::kMapSize> positions;
		std::array<int, Knights::kMapSize + Knights::kMapSize> intensity;
		int stackPos = 0;
		Knights::Vec2i currentPos;

		positions[stackPos] = originalPos;
		intensity[stackPos] = emission;
		++stackPos;

		while (stackPos > 0) {
			--stackPos;

			currentPos = positions[ stackPos ];
			emission = intensity[ stackPos ];

			if (emission <= 2) {
				continue;
			}

			int x = currentPos.x;
			int y = currentPos.y;

			if (!isValid(currentPos)) {
				continue;
			}

			if ( isBlock( occluders, x, y ) ) {
				continue;
			}

			if ( (lightMap[y][x] + emission) <= 255 && (lightMap[y][x] + emission) >= 0 ) {
				lightMap[y][x] += emission;
			} else {
				lightMap[y][x] = 255;
			}

			//The -1 is due to the fact I will add a new element.

			if ( stackPos < positions.size() - 1) {
				positions[stackPos] =  Knights::Vec2i{currentPos.x + 1, currentPos.y};
				intensity[stackPos] = emission / 2;
				++stackPos;
			}

			if ( stackPos < positions.size() - 1) {
				positions[stackPos] =  Knights::Vec2i{currentPos.x, currentPos.y + 1};
				intensity[stackPos] = emission / 2;
				++stackPos;
			}

			if ( stackPos < positions.size() - 1) {
				positions[stackPos] =  Knights::Vec2i{currentPos.x - 1, currentPos.y};
				intensity[stackPos] = emission / 2;
				++stackPos;
			}

			if ( stackPos < positions.size() - 1) {
				positions[stackPos] =  Knights::Vec2i{currentPos.x, currentPos.y - 1};
				intensity[stackPos] = emission / 2;
				++stackPos;
			}
		}
	}
}
