//
// Created by monty on 30/07/16.
//


#ifndef KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H
#define KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H

namespace odb {

	class VisibilityStrategy {

		static bool isValid(Knights::Vec2i pos);
		static void castVisibility(Knights::EDirection from, VisMap &visMap, const IntMap& occluders, Knights::Vec2i pos, Knights::Vec2i offset);
		static bool isBlock(const IntMap& occluders, int x, int y);
		static Knights::EDirection getOposite( Knights::EDirection d );
	public:
		static void castVisibility(VisMap &visMap, const IntMap& occluders, Knights::Vec2i pos, Knights::EDirection direction, bool cleanPrevious);
		static void mergeInto(const VisMap &map1, const VisMap &map2, VisMap &result);
	};
}

#endif //KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H
