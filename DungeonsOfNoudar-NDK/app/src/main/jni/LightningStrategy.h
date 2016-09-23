//
// Created by monty on 30/07/16.
//


#ifndef KNIGHTSOFALENTEJOANDROID_AS_LIGHTNINGSTRATEGY_H
#define KNIGHTSOFALENTEJOANDROID_AS_LIGHTNINGSTRATEGY_H

namespace odb {

	class LightningStrategy {



		enum Direction {
			N,
			E,
			S,
			W,
			TOP,
			BOTTOM
		};

		using Vec2i = std::pair<int, int>;

		static bool isValid(Vec2i pos);

		static void castLight(Direction from, LightMap &lightMap, int emission,
		               IntGameMap occluders, Vec2i pos);

	public:
		static void castPointLight(LightMap &lightMap, int emission, IntGameMap occluders,
		                                       int x, int y);

		static void castLightInAllDirections(LightMap &lightMap, int emission, IntGameMap occluders,
		                                                 int x, int y);
	};
}

#endif //KNIGHTSOFALENTEJOANDROID_AS_LIGHTNINGSTRATEGY_H
