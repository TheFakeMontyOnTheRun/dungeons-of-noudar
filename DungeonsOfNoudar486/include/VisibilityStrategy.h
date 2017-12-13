//
// Created by monty on 30/07/16.
//


#ifndef KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H
#define KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H

namespace odb {
	enum class EVisibility : uint8_t {kVisible, kInvisible};
	using IntMap = array<array< Knights::ElementView, Knights::kMapSize>, Knights::kMapSize >;
	using VisMap = array<array< EVisibility, Knights::kMapSize>, Knights::kMapSize >;

	using DistanceDistribution = array<vector<Knights::Vec2i>, Knights::kMapSize + Knights::kMapSize >;
	class VisibilityStrategy {

		static bool isValid(const Knights::Vec2i& pos);
		static void castVisibility(Knights::EDirection from, VisMap &visMap, const IntMap& occluders, const Knights::Vec2i& pos, const Knights::Vec2i& offset, DistanceDistribution& distances);
		static bool isBlock(const IntMap& occluders, Knights::EDirection from, const Knights::Vec2i& currentPos);
		static bool isVisibleAt(const VisMap& visMap, Knights::EDirection from, const Knights::Vec2i& currentPos );
		static void setIsVisible(VisMap& visMap, Knights::EDirection from, const Knights::Vec2i& currentPos );
	public:
		static void mergeInto(const VisMap &map1, const VisMap &map2, VisMap &result);

        static void makeAllVisible(VisMap& array);
        static Knights::Vec2i transform( Knights::EDirection from, const Knights::Vec2i& currentPos );
        static void castVisibility(VisMap &visMap, const IntMap& occluders, const Knights::Vec2i& pos, Knights::EDirection direction, bool cleanPrevious, DistanceDistribution& distances);
    };
}

#endif //KNIGHTSOFALENTEJOANDROID_AS_VISIBILITYSTRATEGY_H
