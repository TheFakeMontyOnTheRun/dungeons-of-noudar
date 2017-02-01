cmake_minimum_required(VERSION 3.5)
project(dungeons_of_noudar_x11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DMESA_GLES2")

include_directories(../noudar-core/noudar-core/include)
include_directories(../NullSoundSystem)
include_directories(../noudar-rendering)
include_directories(../gles2-renderer)
include_directories(../X11-version)
include_directories(../glm)
include_directories(../nanovg/src)
include_directories(../gsl)

set(SOURCE_FILES
        ../noudar-core/noudar-core/include/commands/CMoveActorCommand.h
        ../noudar-core/noudar-core/include/commands/CActorMeleeAttackCommand.h
        ../noudar-core/noudar-core/include/commands/CEndTurnCommand.h
        ../noudar-core/noudar-core/include/commands/CLoadNewLevelCommand.h
        ../noudar-core/noudar-core/include/commands/CNullCommand.h
        ../noudar-core/noudar-core/include/commands/CQuitCommand.h
        ../noudar-core/noudar-core/include/commands/CTurnActorCommand.h
        ../noudar-core/noudar-core/include/commands/IGameCommand.h
        ../noudar-core/noudar-core/include/commands/CHitscanAttackCommand.h
        ../noudar-core/noudar-core/include/CPlainFileLoader.h
        ../noudar-core/noudar-core/include/CActor.h
        ../noudar-core/noudar-core/include/CElixirFountain.h
        ../noudar-core/noudar-core/include/CMonsterGenerator.h
        ../noudar-core/noudar-core/include/CMonster.h
        ../noudar-core/noudar-core/include/CDoorway.h
        ../noudar-core/noudar-core/include/CGame.h
        ../noudar-core/noudar-core/include/CMap.h
        ../noudar-core/noudar-core/include/IRenderer.h
        ../noudar-core/noudar-core/include/Vec2i.h
        ../noudar-core/noudar-core/include/CTeam.h
        ../noudar-core/noudar-core/include/CCharacterArchetype.h
        ../noudar-core/noudar-core/include/CHuntApproach.h
        ../noudar-core/noudar-core/include/CCharacter.h
        X11-version/main.cpp
        ../noudar-core/noudar-core/src/CActor.cpp
        ../noudar-core/noudar-core/src/CMonster.cpp
        ../noudar-core/noudar-core/src/CTeam.cpp
        ../noudar-core/noudar-core/src/CMap.cpp
        ../noudar-core/noudar-core/src/Common.cpp
        ../noudar-core/noudar-core/src/CHuntApproach.cpp
        ../noudar-core/noudar-core/src/CCharacter.cpp
        ../noudar-core/noudar-core/src/CCharacterArchetype.cpp
        ../noudar-core/noudar-core/src/CElixirFountain.cpp
        ../noudar-core/noudar-core/src/CDoorway.cpp
        ../noudar-core/noudar-core/src/IFileLoaderDelegate.cpp
        ../noudar-core/noudar-core/src/IMapElement.cpp
        ../noudar-core/noudar-core/src/CGame.cpp
        ../noudar-core/noudar-core/src/CPlainFileLoader.cpp
        ../noudar-core/noudar-core/src/CGameDelegate.cpp
        ../noudar-core/noudar-core/src/commands/CActorMeleeAttackCommand.cpp
        ../noudar-core/noudar-core/src/commands/IGameCommand.cpp
        ../noudar-core/noudar-core/src/commands/CMoveActorCommand.cpp
        ../noudar-core/noudar-core/src/commands/CLoadNewLevelCommand.cpp
        ../noudar-core/noudar-core/src/commands/CTurnActorCommand.cpp
        ../noudar-core/noudar-core/src/commands/CQuitCommand.cpp
        ../noudar-core/noudar-core/src/commands/CNullCommand.cpp
        ../noudar-core/noudar-core/src/commands/CEndTurnCommand.cpp
        ../noudar-core/noudar-core/src/commands/CHitscanAttackCommand.cpp
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
        X11-version/LoadPNG.cpp
        X11-version/WindowOperationsX11.cpp
        X11-version/x11_native.cpp
        ../gles2-renderer/Logger.cpp
        ../gles2-renderer/Trig.cpp
        ../gles2-renderer/TrigBatch.cpp
        ../gles2-renderer/VBORenderingJob.cpp
        ../gles2-renderer/NativeBitmap.cpp
        ../gles2-renderer/WavefrontOBJReader.cpp
        ../gles2-renderer/WavefrontMaterialReader.cpp
        ../gles2-renderer/Scene.cpp
        ../gles2-renderer/Texture.cpp
        ../nanovg/src/nanovg.c
        ../NullSoundSystem/SoundClip.cpp
        ../NullSoundSystem/SoundEmitter.cpp
        ../NullSoundSystem/SoundListener.cpp
        ../NullSoundSystem/SoundUtils.cpp)

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

add_executable(dungeons_of_noudar_x11 ${SOURCE_FILES})

target_link_libraries(dungeons_of_noudar_x11 ${OPENGL_LIBRARIES} ${X11_LIBRARIES}  ${PNG_LIBRARY} ${EGL_LIBRARIES} ${GLESV2_LIBRARIES}  )