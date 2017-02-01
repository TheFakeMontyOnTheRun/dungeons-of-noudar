//
// Created by monty on 30/07/16.
//


#ifndef KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H
#define KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H

namespace odb {

	class VisibilityStrategy {

		static bool isValid(Knights::Vec2i pos);
		static void castVisibility(Knights::EDirection from, IntMap &visMap, IntMap& occluders, Knights::Vec2i pos);
		static bool isBlock(IntMap occluders, int x, int y);
		Knights::EDirection getLeftOf( Knights::EDirection d );
		Knights::EDirection getRightOf( Knights::EDirection d );
		Knights::EDirection getOposite( Knights::EDirection d );
		Knights::Vec2i getOffsetFor( Knights::EDirection d, Knights::Vec2i accumulated );
	public:
		static void castVisibility(IntMap &visMap, IntMap& occluders, Knights::Vec2i pos);
	};
}

#endif //KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H
