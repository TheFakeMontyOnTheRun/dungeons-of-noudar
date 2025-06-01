# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(native-lib)

include_directories(../../../../../DungeonsOfNoudar486/include)
include_directories(../../../../../fixed_point/include)
include_directories(../../../../../stb)
include_directories(../../../../../noudar-core/noudar-core/include)
include_directories(../../../../../noudar-rendering)



set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDLSW -std=c90 -DANDROID -DDONT_LOAD_LEVEL_UPON_START -DUSE_ITEMS_INSTANTLY -DVGA   -D_BSD_SOURCE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDLSW -std=c++14 -DANDROID -DDONT_LOAD_LEVEL_UPON_START -DUSE_ITEMS_INSTANTLY -DVGA   -D_BSD_SOURCE")


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).

        ../../../../../DungeonsOfNoudar486/src/CRenderer_Rasterization.cpp
        ../../../../../DungeonsOfNoudar486/src/CRenderer_Tesselation.cpp
        ../../../../../DungeonsOfNoudar486/src/CRenderer.cpp
        ../../../../../DungeonsOfNoudar486/src/NativeBitmap.cpp
        ../../../../../DungeonsOfNoudar486/src/RasterizerCommon.cpp
        ../../../../../DungeonsOfNoudar486/src/CTile3DProperties.cpp
        ../../../../../DungeonsOfNoudar486/src/CPackedFileReader.cpp
        ../../../../../DungeonsOfNoudar486/src/VisibilityStrategy.cpp
        ../../../../../DungeonsOfNoudar486/events.cpp
        ../../../../../noudar-rendering/LoadPNG.cpp
        ../../../../../noudar-rendering/Logger.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CMoveActorCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CEndTurnCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CLoadNewLevelCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CNullCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CQuitCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CTurnActorCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CCycleNextItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CPickItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CDropItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CSuggestItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/CUseCurrentItemCommand.cpp
        ../../../../../noudar-core/noudar-core/src/commands/IGameCommand.cpp
        ../../../../../noudar-core/noudar-core/src/Vec2i.cpp
        ../../../../../noudar-core/noudar-core/src/Common.cpp
        ../../../../../noudar-core/noudar-core/src/CActor.cpp
        ../../../../../noudar-core/noudar-core/src/CItem.cpp
        ../../../../../noudar-core/noudar-core/src/CCharacter.cpp
        ../../../../../noudar-core/noudar-core/src/CMonsterGenerator.cpp
        ../../../../../noudar-core/noudar-core/src/CCharacterArchetype.cpp
        ../../../../../noudar-core/noudar-core/src/CGameDelegate.cpp
        ../../../../../noudar-core/noudar-core/src/CMonster.cpp
        ../../../../../noudar-core/noudar-core/src/IFileLoaderDelegate.cpp
        ../../../../../noudar-core/noudar-core/src/CTeam.cpp
        ../../../../../noudar-core/noudar-core/src/CDoorway.cpp
        ../../../../../noudar-core/noudar-core/src/CGame.cpp
        ../../../../../noudar-core/noudar-core/src/CMap.cpp
        ../../../../../noudar-core/noudar-core/src/IMapElement.cpp
        ../../../../../noudar-core/noudar-core/src/CStorageItem.cpp
        native-lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_library( # Sets the name of the path variable.
        android-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        android )


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
        ${android-lib}

        android
        jnigraphics
        EGL
        OpenSLES
        GLESv2

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})
