CXXFLAGS = -O2 -Werror -Wall -g -c -std=c++0x -ferror-limit=1  -s USE_ZLIB=1 -s USE_LIBPNG=1 -I../OpenALSoundSystem -I../noudar-core/noudar-core/include -Iincludes -I../noudar-rendering -I../gles2-renderer/ -IX11-version -I../glm -I../nanovg/src -s ALLOW_MEMORY_GROWTH=1 -DGLM_FORCE_RADIANS
LDFLAGS =  -O2 -s USE_ZLIB=1 -s USE_LIBPNG=1 -s --preload-file res --use-preload-plugins  -s ALLOW_MEMORY_GROWTH=1
CXX = em++
CC = emcc
OBJS = X11-version/main.o \
    ../noudar-core/noudar-core/src/Vec2i.o \
    ../noudar-core/noudar-core/src/CActor.o \
    ../noudar-core/noudar-core/src/CMonster.o \
    ../noudar-core/noudar-core/src/CMonsterGenerator.o \
    ../noudar-core/noudar-core/src/CTeam.o \
    ../noudar-core/noudar-core/src/CMap.o \
    ../noudar-core/noudar-core/src/CHuntApproach.o \
    ../noudar-core/noudar-core/src/CCharacter.o \
    ../noudar-core/noudar-core/src/CCharacterArchetype.o \
    ../noudar-core/noudar-core/src/CElixirFountain.o \
    ../noudar-core/noudar-core/src/Common.o \
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
    	       ../noudar-rendering/RenderingJobSnapshotAdapter.o \
               ../noudar-rendering/GameNativeAPI.o \
               ../noudar-rendering/OverlayNanoVGRenderer.o \
               ../noudar-rendering/DungeonGLES2Renderer.o \
               ../noudar-rendering/LightningStrategy.o \
               ../noudar-rendering/VisibilityStrategy.o \
               ../noudar-rendering/NoudarDungeonSnapshot.o \
               ../noudar-rendering/SplatAnimation.o \
               ../noudar-rendering/Animation.o \
               ../noudar-rendering/AnimationStep.o \
               ../noudar-rendering/GraphicNode.o \
                ../noudar-rendering/CTile3DProperties.o \
               ../noudar-rendering/NoudarGLES2Bridge.o \
               X11-version/WindowOperationsX11.o \
               X11-version/x11_emscripten.o \
               ../gles2-renderer/Logger.o \
               ../gles2-renderer/CLerp.o \
               ../gles2-renderer/Camera.o \
               ../gles2-renderer/Trig.o \
               ../gles2-renderer/TrigBatch.o \
               ../gles2-renderer/NativeBitmap.o \
               ../gles2-renderer/WavefrontOBJReader.o \
               ../gles2-renderer/VBORenderingJob.o \
               ../gles2-renderer/WavefrontMaterialReader.o \
               ../gles2-renderer/Texture.o \
               ../gles2-renderer/LoadPNG.o \
                ../nanovg/src/nanovg.o \
                ../OpenALSoundSystem/SoundClip.o \
                ../OpenALSoundSystem/SoundEmitter.o \
                ../OpenALSoundSystem/SoundListener.o \
                ../OpenALSoundSystem/SoundUtils.o

TARGET = noudar.html

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	rm *~
	rm *.js
	rm *.mem
	rm *.html
	rm *.data
