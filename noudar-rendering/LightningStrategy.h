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

		using Vec2i = std::pair<int8_t, int8_t>;

		static bool isValid(Vec2i pos);

		static void castLight(Direction from, LightMap &lightMap, int emission,
		               IntMap occluders, Vec2i pos);

	public:
		static void castPointLight(LightMap &lightMap, int emission, IntMap occluders,
		                                       int x, int y);

		static void castLightInAllDirections(LightMap &lightMap, int emission, IntMap occluders,
		                                                 int x, int y);
	};
}

#endif //KNIGHTSOFALENTEJOANDROID_AS_LIGHTNINGSTRATEGY_H
