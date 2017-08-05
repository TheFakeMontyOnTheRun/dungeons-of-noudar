//
// Created by monty on 30/07/16.
//


#ifndef KNIGHTSOFALENTEJOANDROID_AS_LIGHTNINGSTRATEGY_H
#define KNIGHTSOFALENTEJOANDROID_AS_LIGHTNINGSTRATEGY_H

namespace odb {

	class LightningStrategy {



		enum Direction : uint8_t{
			N,
			E,
			S,
			W,
			TOP,
			BOTTOM
		};

		static bool isValid(Knights::Vec2i pos);

		static void castLight(Direction from, LightMap &lightMap, int emission,
		               IntMap occluders, Knights::Vec2i pos);

	public:
		static void castPointLight(LightMap &lightMap, int emission, IntMap occluders,
		                                       int x, int y);
	};
}

#endif //KNIGHTSOFALENTEJOANDROID_AS_LIGHTNINGSTRATEGY_H
