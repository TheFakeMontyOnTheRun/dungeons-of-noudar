//
// Created by monty on 06/12/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H
#define DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H

namespace odb {

	enum class EActorsSnapshotElement {
		kNothing,
		kHeroStanding0,
		kHeroStanding1,
		kHeroDead0,
		kHeroDead1,
		kHeroAttacking0,
		kHeroAttacking1,
		kDemonStanding0,
		kDemonStanding1,
		kDemonDead0,
		kDemonDead1,
		kDemonAttacking0,
		kDemonAttacking1,
	};

	enum class EVisibility {
		kInvisible,
		kVisible,
	};

	using CCharacterId = int;

	using IntMap = std::array<std::array<int, Knights::kMapSize>, Knights::kMapSize>;
	using CharMap = std::array<std::array<EActorsSnapshotElement, Knights::kMapSize>, Knights::kMapSize>;
	using VisMap = std::array<std::array<EVisibility, Knights::kMapSize>, Knights::kMapSize>;
	using AnimationList = std::map<int, std::tuple<glm::vec2, glm::vec2, long> >;

	class NoudarDungeonSnapshot {

	public:
		odb::IntMap map;
		odb::VisMap mVisibilityMap;
		odb::CharMap snapshot;
		odb::IntMap splat;
		odb::IntMap ids;
		odb::IntMap mLightMap;
		Knights::Vec2i mCursorPosition;
		int mCameraId;
		int mTurn;
		long mTimestamp;
		AnimationList movingCharacters;
	};
}
#endif //DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H
