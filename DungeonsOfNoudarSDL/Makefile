CXXFLAGS = -O2 -g -c -std=c++0x -I../noudar-core/noudar-core/include -Iincludes -I../OpenALSoundSystem -I../noudar-rendering -I../gles2-renderer/ -I../nanovg/src -ISDL-version -I../glm `sdl-config --cflags` -ffast-math -fomit-frame-pointer -ferror-limit=1  -DGLM_FORCE_RADIANS
LDFLAGS = -lpng -lopenal -lGL -lGLU `sdl-config --libs`
CXX = clang++
CC = clang
OBJS = SDL-version/main.o \
   ../noudar-core/noudar-core/src/Vec2i.o \
   ../noudar-core/noudar-core/src/CActor.o \
    ../noudar-core/noudar-core/src/CMonster.o \
    ../noudar-core/noudar-core/src/CMonsterGenerator.o \
    ../noudar-core/noudar-core/src/CTeam.o \
    ../noudar-core/noudar-core/src/CMap.o \
    ../noudar-core/noudar-core/src/Common.o \
    ../noudar-core/noudar-core/src/CHuntApproach.o \
    ../noudar-core/noudar-core/src/CCharacter.o \
    ../noudar-core/noudar-core/src/CCharacterArchetype.o \
    ../noudar-core/noudar-core/src/CElixirFountain.o \
    ../noudar-core/noudar-core/src/CDoorway.o \
    ../noudar-core/noudar-core/src/IFileLoaderDelegate.o \
    ../noudar-core/noudar-core/src/IMapElement.o \
    ../noudar-core/noudar-core/src/CGame.o \
    ../noudar-core/noudar-core/src/CItem.o \
    ../noudar-core/noudar-core/src/CStorageItem.o \
    ../noudar-core/noudar-core/src/CPlainFileLoader.o \
    ../noudar-core/noudar-core/src/CGameDelegate.o \
    ../noudar-core/noudar-core/src/commands/IGameCommand.o \
    ../noudar-core/noudar-core/src/commands/CMoveActorCommand.o \
    ../noudar-core/noudar-core/src/commands/CLoadNewLevelCommand.o \
    ../noudar-core/noudar-core/src/commands/CTurnActorCommand.o \
    ../noudar-core/noudar-core/src/commands/CQuitCommand.o \
    ../noudar-core/noudar-core/src/commands/CNullCommand.o \
    ../noudar-core/noudar-core/src/commands/CEndTurnCommand.o \
    ../noudar-core/noudar-core/src/commands/CPickItemCommand.o \
    ../noudar-core/noudar-core/src/commands/CDropItemCommand.o \
    ../noudar-core/noudar-core/src/commands/CUseCurrentItemCommand.o \
    ../noudar-core/noudar-core/src/commands/CCycleNextItemCommand.o \
    ../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.o \
        ../noudar-rendering/DungeonGL11Renderer.o \
        ../noudar-rendering/LightningStrategy.o \
        ../noudar-rendering/VisibilityStrategy.o \
        ../noudar-rendering/RenderingJobSnapshotAdapter.o \
	../noudar-rendering/GameNativeAPI.o \
        ../noudar-rendering/NoudarDungeonSnapshot.o \
        ../noudar-rendering/SplatAnimation.o \
        ../noudar-rendering/CTile3DProperties.o \
        ../noudar-rendering/NoudarGLES2Bridge.o \
        SDL-version/WindowOperationsSDL.o \
        ../gles2-renderer/Logger.o \
        ../gles2-renderer/Trig.o \
        ../gles2-renderer/Camera.o \
        ../gles2-renderer/LoadPNG.o \
        ../nanovg/src/nanovg.o \
        ../gles2-renderer/CLerp.o \
        ../gles2-renderer/Scene.o \
        ../gles2-renderer/WavefrontOBJReader.o \
        ../gles2-renderer/WavefrontMaterialReader.o \
        ../gles2-renderer/TrigBatch.o \
        ../gles2-renderer/VBORenderingJob.o \
        ../gles2-renderer/NativeBitmap.o \
        ../gles2-renderer/Texture.o \
        ../OpenALSoundSystem/SoundClip.o \
        ../OpenALSoundSystem/SoundEmitter.o \
        ../OpenALSoundSystem/SoundListener.o \
        ../OpenALSoundSystem/SoundUtils.o

TARGET = noudar

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	rm *~
