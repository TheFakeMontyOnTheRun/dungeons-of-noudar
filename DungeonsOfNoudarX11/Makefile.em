CXXFLAGS = -O2 -g -c -std=c++0x -ferror-limit=1  -s USE_ZLIB=1 -s USE_LIBPNG=1 -I../OpenALSoundSystem -I../noudar-core/noudar-core/include -Iincludes -I../noudar-rendering -I../gles2-renderer/ -IX11-version -I../glm
LDFLAGS =  -O2 -s USE_ZLIB=1 -s USE_LIBPNG=1 -s --preload-file res --use-preload-plugins
CXX = em++
OBJS = X11-version/main.o \
               ../noudar-core/noudar-core/src/CActor.o \
               ../noudar-core/noudar-core/src/CBaphomet.o \
               ../noudar-core/noudar-core/src/CBullKnight.o \
               ../noudar-core/noudar-core/src/CCuco.o \
               ../noudar-core/noudar-core/src/CDemon.o \
               ../noudar-core/noudar-core/src/CFalconKnight.o \
               ../noudar-core/noudar-core/src/CMap.o \
               ../noudar-core/noudar-core/src/CGameDelegate.o \
               ../noudar-core/noudar-core/src/CMoura.o \
               ../noudar-core/noudar-core/src/CTurtleKnight.o \
               ../noudar-core/noudar-core/src/CDoorway.o \
               ../noudar-core/noudar-core/src/IMapElement.o \
               ../noudar-core/noudar-core/src/CGame.o \
               ../noudar-rendering/GameNativeAPI.o \
               ../noudar-rendering/DungeonGLES2Renderer.o \
               ../noudar-rendering/LightningStrategy.o \
               ../noudar-rendering/NoudarGLES2Bridge.o \
               X11-version/LoadPNG.o \
               X11-version/WindowOperationsX11.o \
               X11-version/x11_emscripten.o \
               ../gles2-renderer/Logger.o \
               ../gles2-renderer/Trig.o \
               ../gles2-renderer/TrigBatch.o \
               ../gles2-renderer/NativeBitmap.o \
               ../gles2-renderer/Texture.o \
                ../gles2-renderer/Common.o \
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
