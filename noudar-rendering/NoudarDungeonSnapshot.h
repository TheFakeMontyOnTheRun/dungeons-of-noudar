//
// Created by monty on 06/12/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H
#define DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H

namespace odb {

	enum class EActorsSnapshotElement : uint8_t {
		kNothing,
		kHeroStanding0,
		kHeroStanding1,
		kHeroAttacking0,
		kHeroAttacking1,
        kWeakenedDemonAttacking0,
		kWeakenedDemonAttacking1,
		kWeakenedDemonStanding0,
		kWeakenedDemonStanding1,
		kStrongDemonAttacking0,
		kStrongDemonAttacking1,
		kStrongDemonStanding0,
		kStrongDemonStanding1,
		kCocoonStanding0,
		kCocoonStanding1,
		kEvilSpiritAttacking0,
		kEvilSpiritAttacking1,
		kEvilSpiritStanding0,
		kEvilSpiritStanding1,
		kWarthogAttacking0,
		kWarthogAttacking1,
		kWarthogStanding0,
		kWarthogStanding1,
		kMonkAttacking0,
		kMonkAttacking1,
		kMonkStanding0,
		kMonkStanding1,
		kFallenAttacking0,
		kFallenAttacking1,
		kFallenStanding0,
		kFallenStanding1,
		kRope,
		kMagicSeal
	};

	enum class EVisibility : uint8_t {
		kInvisible,
		kVisible,
	};

	using CCharacterId = int8_t;

	using IntMap = array<array<int8_t , Knights::kMapSize>, Knights::kMapSize>;
	using CharMap = array<array<EActorsSnapshotElement, Knights::kMapSize>, Knights::kMapSize>;
	using VisMap = array<array<EVisibility, Knights::kMapSize>, Knights::kMapSize>;
    using LightMap = array<array<int , Knights::kMapSize>, Knights::kMapSize>;
	using AnimationList = std::map<int8_t, std::tuple<glm::vec2, glm::vec2, long> >;

	class NoudarDungeonSnapshot {

	public:
		odb::IntMap map;
		odb::VisMap mVisibilityMap;
		odb::CharMap snapshot;
		odb::IntMap splat;
		odb::IntMap ids;
#ifndef OSMESA
        odb::LightMap mLightMap;
#endif
		odb::IntMap mItemMap;
		Knights::Vec2i mCursorPosition;
		int mCameraId;
		int mTurn;
		long mTimestamp;
		AnimationList movingCharacters;
		std::string mCurrentItem = "";
		int mHP;
        Knights::Vec2i mCameraPosition;
	};

	std::ostream& operator<<(std::ostream& os, const NoudarDungeonSnapshot& aSnapshot );
	std::string to_string( const NoudarDungeonSnapshot& aSnapshot );

}
#endif //DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H
