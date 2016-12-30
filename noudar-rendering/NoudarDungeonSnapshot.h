//
// Created by monty on 06/12/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H
#define DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H

namespace odb {

    using IntMap = std::array<std::array<int, Knights::kMapSize>, Knights::kMapSize>;
    using CharMap = std::array<std::array<char, Knights::kMapSize>, Knights::kMapSize>;

    class NoudarDungeonSnapshot {

    public:
        odb::CharMap map;
        odb::CharMap snapshot;
        odb::IntMap splat;
        odb::IntMap ids;
    };
}
#endif //DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H
