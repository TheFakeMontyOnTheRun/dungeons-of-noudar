cmake_minimum_required(VERSION 3.8)
project(DungeonsOfNoudar486)

find_package(SDL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_ITEMS_INSTANTLY -DVGA")
set(CMAKE_CXX_STANDARD 14)
include_directories(include)
include_directories(../stb)
include_directories(../fixed_point/include)
include_directories(../noudar-core/noudar-core/include)
include_directories(../noudar-core/EASTL/include )
include_directories(../noudar-core/EASTL/test/packages/EABase/include/Common)

include_directories(../noudar-rendering)
include_directories(../gles2-renderer)

set(SOURCE_FILES
        SDLVersion/CSDLRenderer.cpp
        SDLVersion/main.cpp
        src/CRenderer.cpp
        src/NativeBitmap.cpp
        src/RasterizerCommon.cpp
        src/CTile3DProperties.cpp
        include/RasterizerCommon.h
        include/CRenderer.h
        include/RaycastCommon.h
        ../noudar-core/noudar-core/include/commands/CMoveActorCommand.h
        ../noudar-core/noudar-core/include/commands/CEndTurnCommand.h
        ../noudar-core/noudar-core/include/commands/CLoadNewLevelCommand.h
        ../noudar-core/noudar-core/include/commands/CNullCommand.h
        ../noudar-core/noudar-core/include/commands/CUseCurrentItemCommand.h
        ../noudar-core/noudar-core/include/commands/CQuitCommand.h
        ../noudar-core/noudar-core/include/commands/CTurnActorCommand.h
        ../noudar-core/noudar-core/include/commands/CDropItemCommand.h
        ../noudar-core/noudar-core/include/commands/CPickItemCommand.h
        ../noudar-core/noudar-core/include/commands/IGameCommand.h
        ../noudar-core/noudar-core/include/CPlainFileLoader.h
        ../noudar-core/noudar-core/include/CActor.h
        ../noudar-core/noudar-core/include/CMonsterGenerator.h
        ../noudar-core/noudar-core/include/CMonster.h
        ../noudar-core/noudar-core/include/CDoorway.h
        ../noudar-core/noudar-core/include/CGame.h
        ../noudar-core/noudar-core/include/CMap.h
        ../noudar-core/noudar-core/include/IRenderer.h
        ../noudar-core/noudar-core/include/Vec2i.h
        ../noudar-core/noudar-core/include/CTeam.h
        ../noudar-core/noudar-core/include/CCharacterArchetype.h
        ../noudar-core/noudar-core/include/CCharacter.h
        ../noudar-core/noudar-core/src/commands/CMoveActorCommand.cpp
        ../noudar-core/noudar-core/src/commands/CEndTurnCommand.cpp
        ../noudar-core/noudar-core/src/commands/CLoadNewLevelCommand.cpp
        ../noudar-core/noudar-core/src/commands/CNullCommand.cpp
        ../noudar-core/noudar-core/src/commands/CQuitCommand.cpp
        ../noudar-core/noudar-core/src/commands/CTurnActorCommand.cpp
        ../noudar-core/noudar-core/src/commands/CCycleNextItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CPickItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CDropItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CUseCurrentItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/IGameCommand.cpp
        ../noudar-core/noudar-core/src/CPlainFileLoader.cpp
        ../noudar-core/noudar-core/src/Vec2i.cpp
        ../noudar-core/noudar-core/src/Common.cpp
        ../noudar-core/noudar-core/src/CActor.cpp
        ../noudar-core/noudar-core/src/CItem.cpp
        ../noudar-core/noudar-core/include/CItem.h
        ../noudar-core/noudar-core/src/CCharacter.cpp
        ../noudar-core/noudar-core/src/CMonsterGenerator.cpp
        ../noudar-core/noudar-core/include/CMonsterGenerator.h
        ../noudar-core/noudar-core/src/CCharacterArchetype.cpp
        ../noudar-core/noudar-core/src/CGameDelegate.cpp
        ../noudar-core/noudar-core/src/CMonster.cpp
        ../noudar-core/noudar-core/src/IFileLoaderDelegate.cpp
        ../noudar-core/noudar-core/src/CTeam.cpp
        ../noudar-core/noudar-core/src/CDoorway.cpp
        ../noudar-core/noudar-core/src/CGame.cpp
        ../noudar-core/noudar-core/src/CMap.cpp
        ../noudar-core/noudar-core/src/IMapElement.cpp
        ../noudar-core/noudar-core/src/CStorageItem.cpp
        ../noudar-core/noudar-core/include/CStorageItem.h
        ../gles2-renderer/Logger.cpp
        ../gles2-renderer/LoadPNG.cpp
        )

find_package(OpenGL REQUIRED)

include (FindPkgConfig)

add_executable(DungeonsOfNoudar486 ${SOURCE_FILES})

target_link_libraries(DungeonsOfNoudar486 ${OPENGL_LIBRARIES} ${SDL_LIBRARY}   )