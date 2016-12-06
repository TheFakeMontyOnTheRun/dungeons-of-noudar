//
// Created by monty on 06/12/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H
#define DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H

namespace odb {
    class NoudarDungeonSnapshot {
    public:
        odb::IntGameMap map;
        odb::IntGameMap snapshot;
        odb::IntGameMap splat;
        odb::IntField ids;
    };
}
#endif //DUNGEONSOFNOUDAR_NDK_NOUDARDUNGEONSNAPSHOT_H
