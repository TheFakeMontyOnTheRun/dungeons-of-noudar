CXXFLAGS = -O2 -g -c -std=c++0x -ferror-limit=1 -I../noudar-core/noudar-core/include -Iincludes -I../OpenALSoundSystem -I../noudar-rendering -I../gles2-renderer/ -ISDL-version -I../glm `sdl-config --cflags`
LDFLAGS = -lpng -lopenal -lGL -lGLU `sdl-config --libs`
CXX = clang++
OBJS = SDL-version/main.o \
        ../noudar-core/noudar-core/src/CActor.o \
        ../noudar-core/noudar-core/src/CKnight.o \
        ../noudar-core/noudar-core/src/CPlainFileLoader.o \
        ../noudar-core/noudar-core/src/IFileLoaderDelegate.o \
        ../noudar-core/noudar-core/src/CCuco.o \
        ../noudar-core/noudar-core/src/CMap.o \
        ../noudar-core/noudar-core/src/CElixirFountain.o \
        ../noudar-core/noudar-core/src/CDoorway.o \
        ../noudar-core/noudar-core/src/IMapElement.o \
        ../noudar-core/noudar-core/src/CGame.o \
        ../noudar-core/noudar-core/src/CGameDelegate.o \
        ../noudar-core/noudar-core/src/commands/CActorMeleeAttackCommand.o \
        ../noudar-core/noudar-core/src/commands/IGameCommand.o \
        ../noudar-core/noudar-core/src/commands/CMoveActorCommand.o \
        ../noudar-core/noudar-core/src/commands/CLoadNewLevelCommand.o \
        ../noudar-core/noudar-core/src/commands/CTurnActorCommand.o \
        ../noudar-core/noudar-core/src/commands/CQuitCommand.o \
        ../noudar-core/noudar-core/src/commands/CNullCommand.o \
        ../noudar-core/noudar-core/src/commands/CEndTurnCommand.o \
        ../noudar-rendering/GameNativeAPI.o \
        ../noudar-rendering/DungeonGL11Renderer.o \
        ../noudar-rendering/LightningStrategy.o \
        ../noudar-rendering/NoudarDungeonSnapshot.o \
        ../noudar-rendering/SplatAnimation.o \
        ../noudar-rendering/NoudarGLES2Bridge.o \
        SDL-version/LoadPNG.o \
        SDL-version/WindowOperationsSDL.o \
        ../gles2-renderer/Logger.o \
        ../gles2-renderer/Trig.o \
        ../gles2-renderer/TrigBatch.o \
        ../gles2-renderer/VBORenderingJob.o \
        ../gles2-renderer/NativeBitmap.o \
        ../gles2-renderer/Texture.o \
        ../gles2-renderer/Common.o \
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
