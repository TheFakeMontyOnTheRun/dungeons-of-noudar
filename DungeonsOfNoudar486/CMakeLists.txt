cmake_minimum_required(VERSION 3.8)
project(DungeonsOfNoudar486)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_find")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDONT_LOAD_LEVEL_UPON_START -DUSE_ITEMS_INSTANTLY -DVGA -DSDLSW")


find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})

include_directories(include)
include_directories(../fixed_point/include)
include_directories(../noudar-core/noudar-core/include)
include_directories(../stb)
include_directories(../noudar-rendering)

set(SOURCE_FILES
        SDLVersion/CSDLRenderer.cpp
        src/CRenderer.cpp
        src/CRenderer_Rasterization.cpp
        src/CRenderer_Tesselation.cpp
        src/NativeBitmap.cpp
        src/RasterizerCommon.cpp
        src/CTile3DProperties.cpp
        src/VisibilityStrategy.cpp
        include/RasterizerCommon.h
        include/CRenderer.h
        include/RaycastCommon.h
        main.cpp
        events.cpp
        ../noudar-core/noudar-core/include/commands/CMoveActorCommand.h
        ../noudar-core/noudar-core/include/commands/CEndTurnCommand.h
        ../noudar-core/noudar-core/include/commands/CLoadNewLevelCommand.h
        ../noudar-core/noudar-core/include/commands/CNullCommand.h
        ../noudar-core/noudar-core/include/commands/CUseCurrentItemCommand.h
        ../noudar-core/noudar-core/include/commands/CQuitCommand.h
        ../noudar-core/noudar-core/include/commands/CTurnActorCommand.h
        ../noudar-core/noudar-core/include/commands/CDropItemCommand.h
        ../noudar-core/noudar-core/include/commands/CPickItemCommand.h
        ../noudar-core/noudar-core/include/commands/CSuggestItemCommand.h
        ../noudar-core/noudar-core/include/commands/IGameCommand.h
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
        ../noudar-core/noudar-core/src/commands/CSuggestItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/CUseCurrentItemCommand.cpp
        ../noudar-core/noudar-core/src/commands/IGameCommand.cpp
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
        OPL2LPT/MusicSDL.cpp
        ../noudar-rendering/Logger.cpp
        ../noudar-rendering/LoadPNG.cpp
        src/CPackedFileReader.cpp
        include/CPackedFileReader.h
        src/CPackedFileReader.cpp
        include/CPackedFileReader.h)

include (FindPkgConfig)

add_executable(DungeonsOfNoudar486 ${SOURCE_FILES} )

target_include_directories(DungeonsOfNoudar486 PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(DungeonsOfNoudar486 ${SDL2_LIBRARY}  m )