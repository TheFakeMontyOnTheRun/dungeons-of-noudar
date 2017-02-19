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

#include "LightningStrategy.h"

namespace odb {

	bool LightningStrategy::isValid(Vec2i pos) {
		return 0 <= pos.first && pos.first < 20 && 0 <= pos.second && pos.second < 20;
	}

	void LightningStrategy::castPointLight(IntMap &lightMap, int emission, IntMap occluders,
	                                       int x, int y) {
		castLight(Direction::TOP, lightMap, emission, occluders, Vec2i{x, y});
	}

	void LightningStrategy::castLightInAllDirections(IntMap &lightMap, int emission, IntMap occluders,
	                                                 int x, int y) {

		castLight(Direction::N, lightMap, emission, occluders, Vec2i{x, y - 1});
		castLight(Direction::E, lightMap, emission, occluders, Vec2i{x + 1, y});
		castLight(Direction::S, lightMap, emission, occluders, Vec2i{x, y + 1});
		castLight(Direction::W, lightMap, emission, occluders, Vec2i{x - 1, y});
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


	void LightningStrategy::castLight(Direction from, IntMap &lightMap, int emission,
	                                  IntMap occluders, Vec2i pos) {

		if (emission <= 1) {
			return;
		}

		int x = pos.first;
		int y = pos.second;

		if (!isValid(pos)) {
			return;
		}

		if ( isBlock( occluders, x, y ) ) {
			return;
		}


		if ( lightMap[y][x] + emission <= 255 ) {
			lightMap[y][x] += emission;
		} else {
			lightMap[y][x] = 255;
		}

		castLight(Direction::N, lightMap, (from == Direction::N ? 0 : emission / 2), occluders,
		          Vec2i{x, y - 1});

		castLight(Direction::W, lightMap, (from == Direction::W ? 0 : emission / 2), occluders,
		          Vec2i{x - 1, y});

		castLight(Direction::S, lightMap, (from == Direction::S ? 0 : emission / 2), occluders,
		          Vec2i{x, y + 1});

		castLight(Direction::E, lightMap, (from == Direction::E ? 0 : emission / 2), occluders,
		          Vec2i{x + 1, y});
	}
}
