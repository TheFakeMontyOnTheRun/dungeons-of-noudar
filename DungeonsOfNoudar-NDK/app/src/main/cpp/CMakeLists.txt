cmake_minimum_required(VERSION 3.4.1)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -g -Ofast -fno-inline -DUSE_ITEMS_INSTANTLY -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -std=gnu++14 -g -Ofast -fno-inline -DUSE_ITEMS_INSTANTLY -Wall")


include_directories("../../../../../glm")
include_directories("../../../../../stb")
include_directories("../../../../../noudar-core/noudar-core/include")
include_directories("../../../../../noudar-rendering")
include_directories("../../../../../NullSoundSystem")

add_library(NdkGlue SHARED
        android_asset_operations.cpp
        AndroidFileLoaderDelegate.cpp
        NdkGlue.cpp
        ../../../../../noudar-core/noudar-core/include/commands/CPickItemCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CDropItemCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CUseCurrentItemCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CCycleNextItemCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CCyclePreviousItemCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CMoveActorCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CEndTurnCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CLoadNewLevelCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CNullCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CQuitCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/CTurnActorCommand.h
        ../../../../../noudar-core/noudar-core/include/commands/IGameCommand.h
        ../../../../../noudar-core/noudar-core/include/CActor.h
        ../../../../../noudar-core/noudar-core/include/CMonster.h
        ../../../../../noudar-core/noudar-core/include/CMonsterGenerator.h
        ../../../../../noudar-core/noudar-core/include/CDoorway.h
        ../../../../../noudar-core/noudar-core/include/CGame.h
        ../../../../../noudar-core/noudar-core/include/CItem.h
        ../../../../../noudar-core/noudar-core/include/CMap.h
        ../../../../../noudar-core/noudar-core/include/IRenderer.h
        ../../../../../noudar-core/noudar-core/include/Vec2i.h
        ../../../../../noudar-core/noudar-core/include/CTeam.h
        ../../../../../noudar-core/noudar-core/include/CCharacterArchetype.h
        ../../../../../noudar-core/noudar-core/include/CCharacter.h
        ../../../../../noudar-core/noudar-core/src/Vec2i.cpp
        ../../../../../noudar-core/noudar-core/src/CActor.cpp
        ../../../../../noudar-core/noudar-core/src/CMonster.cpp
        ../../../../../noudar-core/noudar-core/src/CMonsterGenerator.cpp
        ../../../../../noudar-core/noudar-core/src/CTeam.cpp
        ../../../../../noudar-core/noudar-core/src/CMap.cpp
        ../../../../../noudar-core/noudar-core/src/Common.cpp
        ../../../../../noudar-core/noudar-core/src/CCharacter.cpp
        ../../../../../noudar-core/noudar-core/src/CCharacterArchetype.cpp
        ../../../../../noudar-core/noudar-core/src/CDoorway.cpp
        ../../../../../noudar-core/noudar-core/src/IFileLoaderDelegate.cpp
        ../../../../../noudar-core/noudar-core/src/IMapElement.cpp
        ../../../../../noudar-core/noudar-core/src/CGame.cpp
        ../../../../../noudar-core/noudar-core/src/CItem.cpp
        ../../../../../noudar-core/noudar-core/src/CStorageItem.cpp
        ../../../../../noudar-core/noudar-core/src/CGameDelegate.cpp
        ../../../../../noudar-core/noudar-core/src/commands/IGameCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CMoveActorCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CLoadNewLevelCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CTurnActorCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CQuitCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CNullCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CSuggestItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CEndTurnCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CPickItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CDropItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CUseCurrentItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CCycleNextItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.cpp
        ../../../../../noudar-rendering/RenderingJobSnapshotAdapter.h
        ../../../../../noudar-rendering/RenderingJobSnapshotAdapter.cpp
        ../../../../../noudar-rendering/GameNativeAPI.cpp
        ../../../../../noudar-rendering/DungeonGLES2Renderer.cpp
        ../../../../../noudar-rendering/LightningStrategy.cpp
        ../../../../../noudar-rendering/VisibilityStrategy.h
        ../../../../../noudar-rendering/VisibilityStrategy.cpp
        ../../../../../noudar-rendering/NoudarDungeonSnapshot.cpp
        ../../../../../noudar-rendering/SplatAnimation.cpp
        ../../../../../noudar-rendering/CTile3DProperties.cpp
        ../../../../../noudar-rendering/NoudarGLES2Bridge.cpp
        ../../../../../noudar-rendering/LoadPNG.cpp
        ../../../../../noudar-rendering/CLerp.cpp
        ../../../../../noudar-rendering/CLerp.h
        ../../../../../noudar-rendering/Camera.cpp
        ../../../../../noudar-rendering/Camera.h
        ../../../../../noudar-rendering/Logger.cpp
        ../../../../../noudar-rendering/Camera.cpp
        ../../../../../noudar-rendering/Camera.h
        ../../../../../noudar-rendering/VBORenderingJob.cpp
        ../../../../../noudar-rendering/NativeBitmap.cpp
        ../../../../../NullSoundSystem/SoundClip.cpp
        ../../../../../NullSoundSystem/SoundEmitter.cpp
        ../../../../../NullSoundSystem/SoundListener.cpp
        ../../../../../NullSoundSystem/SoundUtils.cpp
        )

# add lib dependencies
target_link_libraries(NdkGlue
        android
        jnigraphics
        log
        EGL
        OpenSLES
        GLESv2)

