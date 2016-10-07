//
// Created by monty on 30/07/16.
//

#include <glm/glm.hpp>

#include <memory>
#include <iostream>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <map>
#include <vector>
#include "gles2-renderer/NativeBitmap.h"
#include "gles2-renderer/Texture.h"
#include "gles2-renderer/Material.h"
#include "gles2-renderer/Trig.h"
#include "gles2-renderer/TrigBatch.h"
#include "gles2-renderer/MeshObject.h"
#include "gles2-renderer/MaterialList.h"
#include "gles2-renderer/Scene.h"

#include "gles2-renderer/PETEntry.h"
#include "gles2-renderer/PETTable.h"
#include "gles2-renderer/GEOMap.h"

#include "DungeonGLES2Renderer.h"
#include "LightningStrategy.h"

namespace odb {

	bool LightningStrategy::isValid(Vec2i pos) {
		return 0 <= pos.first && pos.first < 20 && 0 <= pos.second && pos.second < 20;
	}

	void LightningStrategy::castPointLight(LightMap &lightMap, int emission, IntGameMap occluders,
	                                       int x, int y) {
		castLight(Direction::TOP, lightMap, emission, occluders, Vec2i{x, y});
	}

	void LightningStrategy::castLightInAllDirections(LightMap &lightMap, int emission, IntGameMap occluders,
	                                                 int x, int y) {

		castLight(Direction::N, lightMap, emission, occluders, Vec2i{x, y - 1});
		castLight(Direction::E, lightMap, emission, occluders, Vec2i{x + 1, y});
		castLight(Direction::S, lightMap, emission, occluders, Vec2i{x, y + 1});
		castLight(Direction::W, lightMap, emission, occluders, Vec2i{x - 1, y});
	}

	bool isBlock(IntGameMap occluders, int x, int y) {

		ETextures tile = occluders[y][x];

		for (auto candidate : {ETextures::Bricks, ETextures::BricksCandles, ETextures::BricksBlood,
		                       ETextures::Begin, ETextures::Exit}) {
			if (candidate == tile) {
				return true;
			}
		}

		return false;
	}


	void LightningStrategy::castLight(Direction from, LightMap &lightMap, int emission,
	                                  IntGameMap occluders, Vec2i pos) {

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
