cmake_minimum_required(VERSION 3.8)
project(dungeons_of_noudar_x11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMESA_GLES2 -DGLM_FORCE_RADIANS -DUSE_ITEMS_INSTANTLY ")
set(CMAKE_CXX_STANDARD 14)
include_directories(../noudar-core/noudar-core/include)
include_directories(../NullSoundSystem)
include_directories(../noudar-rendering)
include_directories(../gles2-renderer)
include_directories(../X11-version)
include_directories(../glm)
include_directories(../nanovg/src)
include_directories(../noudar-core/EASTL/include )
include_directories(../noudar-core/EASTL/test/packages/EABase/include/Common)

set(TEST_SOURCE_FILES
        ../tests/src/TestVisibilityStrategy.cpp
        ../tests/src/TestVec2i.cpp
        ../noudar-core/lib/googletest/googlemock/src/gmock-all.cc
        ../noudar-core/lib/googletest/googlemock/src/gmock_main.cc
        ../noudar-core/lib/googletest/googletest/src/gtest-all.cc)

include_directories(../noudar-core/lib/googletest/googletest/include)
include_directories(../noudar-core/lib/googletest/googlemock/include)
include_directories(../noudar-core/lib/googletest/googletest)
include_directories(../noudar-core/lib/googletest/googlemock)

set(SOURCE_FILES
        ../noudar-core/noudar-core/include/commands/CPickItemCommand.h
        ../noudar-core/noudar-core/include/commands/CDropItemCommand.h
        ../noudar-core/noudar-core/include/commands/CUseCurrentItemCommand.h
        ../noudar-core/noudar-core/include/commands/CCycleNextItemCommand.h
        ../noudar-core/noudar-core/include/commands/CCyclePreviousItemCommand.h
        ../noudar-core/noudar-core/include/commands/CMoveActorCommand.h
        ../noudar-core/noudar-core/include/commands/CEndTurnCommand.h
        ../noudar-core/noudar-core/include/commands/CLoadNewLevelCommand.h
        ../noudar-core/noudar-core/include/commands/CNullCommand.h
        ../noudar-core/noudar-core/include/commands/CQuitCommand.h
        ../noudar-core/noudar-core/include/commands/CTurnActorCommand.h
        ../noudar-core/noudar-core/include/commands/IGameCommand.h
        ../noudar-core/noudar-core/include/CPlainFileLoader.h
        ../noudar-core/noudar-core/include/CActor.h
        ../noudar-core/noudar-core/include/CMonster.h
        ../noudar-core/noudar-core/include/CMonsterGenerator.h
        ../noudar-core/noudar-core/include/CDoorway.h
        ../noudar-core/noudar-core/include/CGame.h
        ../noudar-core/noudar-core/include/CItem.h
        ../noudar-core/noudar-core/include/CMap.h
        ../noudar-core/noudar-core/include/IRenderer.h
        ../noudar-core/noudar-core/include/Vec2i.h
        ../noudar-core/noudar-core/include/CTeam.h
        ../noudar-core/noudar-core/include/CCharacterArchetype.h
        ../noudar-core/noudar-core/include/CCharacter.h
        ../noudar-core/noudar-core/src/Vec2i.cpp
        ../noudar-core/noudar-core/src/CActor.cpp
        ../noudar-core/noudar-core/src/CMonster.cpp
        ../noudar-core/noudar-core/src/CMonsterGenerator.cpp
        ../noudar-core/noudar-core/src/CTeam.cpp
        ../noudar-core/noudar-core/src/CMap.cpp
        ../noudar-core/noudar-core/src/Common.cpp
        ../noudar-core/noudar-core/src/CCharacter.cpp
        ../noudar-core/noudar-core/src/CCharacterArchetype.cpp
        ../noudar-core/noudar-core/src/CDoorway.cpp
        ../noudar-core/noudar-core/src/IFileLoaderDelegate.cpp
        ../noudar-core/noudar-core/src/IMapElement.cpp
        ../noudar-core/noudar-core/src/CGame.cpp
        ../noudar-core/noudar-core/src/CItem.cpp
        ../noudar-core/noudar-core/src/CStorageItem.cpp
        ../noudar-core/noudar-core/src/CPlainFileLoader.cpp
        ../noudar-core/noudar-core/src/CGameDelegate.cpp
        ../noudar-core/noudar-core/src/commands/IGameCommand.cpp
        ../noudar-core/noudar-core/src/commands/CMoveActorCommand.cpp
        ../noudar-core/noudar-core/src/commands/CLoadNewLevelCommand.cpp
        ../noudar-core/noudar-core/src/commands/CTurnActorCommand.cpp
        ../noudar-core/noudar-core/src/commands/CQuitCommand.cpp
        ../noudar-core/noudar-core/src/commands/CNullCommand.cpp
        ../noudar-core/noudar-core/src/commands/CEndTurnCommand.cpp
        ../noudar-core/noudar-core/src/commands/CPickItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CDropItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CUseCurrentItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CCycleNextItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.cpp
        ../noudar-rendering/RenderingJobSnapshotAdapter.h
        ../noudar-rendering/RenderingJobSnapshotAdapter.cpp
        ../noudar-rendering/GameNativeAPI.cpp
        ../noudar-rendering/OverlayNanoVGRenderer.cpp
        ../noudar-rendering/DungeonGLES2Renderer.cpp
        ../noudar-rendering/LightningStrategy.cpp
        ../noudar-rendering/VisibilityStrategy.h
        ../noudar-rendering/VisibilityStrategy.cpp
        ../noudar-rendering/NoudarDungeonSnapshot.cpp
        ../noudar-rendering/SplatAnimation.cpp
        ../noudar-rendering/CTile3DProperties.cpp
        ../noudar-rendering/NoudarGLES2Bridge.cpp
        X11-version/WindowOperationsX11.cpp
        X11-version/x11_native.cpp
        ../gles2-renderer/LoadPNG.cpp
        ../gles2-renderer/CLerp.cpp
        ../gles2-renderer/CLerp.h
        ../gles2-renderer/Camera.cpp
        ../gles2-renderer/Camera.h
        ../gles2-renderer/Logger.cpp
        ../gles2-renderer/Trig.cpp
        ../gles2-renderer/Camera.cpp
        ../gles2-renderer/Camera.h
        ../gles2-renderer/TrigBatch.cpp
        ../gles2-renderer/VBORenderingJob.cpp
        ../gles2-renderer/NativeBitmap.cpp
        ../gles2-renderer/Scene.cpp
        ../nanovg/src/nanovg.c
        ../NullSoundSystem/SoundClip.cpp
        ../NullSoundSystem/SoundEmitter.cpp
        ../NullSoundSystem/SoundListener.cpp
        ../NullSoundSystem/SoundUtils.cpp
        ../noudar-rendering/GraphicNode.cpp
        ../noudar-rendering/GraphicNode.h
        ../noudar-rendering/Animation.cpp
        ../noudar-rendering/Animation.h
        ../noudar-rendering/AnimationStep.cpp
        ../noudar-rendering/AnimationStep.h
        )

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(PNG REQUIRED)

include (FindPkgConfig)

pkg_check_modules (EGL egl)
pkg_check_modules (GLESV2 glesv2)

include_directories(
        ${EGL_INCLUDE_DIRS}
        ${GLESV2_INCLUDE_DIRS}
        ${PNG_INCLUDE_DIR}
)

add_executable(dungeons_of_noudar_x11 ${SOURCE_FILES} X11-version/main.cpp )
add_executable(dungeons_of_noudar_tests ${SOURCE_FILES} ${TEST_SOURCE_FILES} )

target_link_libraries(dungeons_of_noudar_x11 ${OPENGL_LIBRARIES} ${X11_LIBRARIES}  ${PNG_LIBRARY} ${EGL_LIBRARIES} ${GLESV2_LIBRARIES}  )

target_link_libraries(dungeons_of_noudar_tests ${OPENGL_LIBRARIES} ${X11_LIBRARIES}  ${PNG_LIBRARY} ${EGL_LIBRARIES} ${GLESV2_LIBRARIES} pthread )
