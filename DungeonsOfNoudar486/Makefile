CXX = clang++

SDL_LIB = `sdl-config --libs` -lSDL_gfx -lSDL_image -lSDL_mixer
SDL_INCLUDE =  `sdl-config --cflags `

CXXFLAGS = -std=c++14 -O2 -g $(SDL_INCLUDE) -g -DSDLSW -DVGA -DEA_PLATFORM_LINUX -Iinclude -c -Istb -fpermissive -Ofast  -fomit-frame-pointer -fno-exceptions -ffast-math -I../fixed_point/include -IDOS-version -Iinclude -I../stb -I../noudar-core/noudar-core/include  -I../noudar-core/EASTL/include -I../noudar-core/EASTL/test/packages/EABase/include/Common -I../noudar-rendering -I../gles2-renderer -Wno-narrowing -DUSE_ITEMS_INSTANTLY

OBJS = main.o \
        SDLVersion/CSDLRenderer.o \
	    OPL2LPT/MusicSDL.o \
        src/CRenderer.o \
        src/RasterizerCommon.o \
        src/NativeBitmap.o \
        src/CTile3DProperties.o \
	src/CPackedFileReader.o \
	src/VisibilityStrategy.o \
	../gles2-renderer/Logger.o \
        ../gles2-renderer/LoadPNG.o \
        ../noudar-core/noudar-core/src/Vec2i.o \
        ../noudar-core/noudar-core/src/CActor.o \
        ../noudar-core/noudar-core/src/CMonster.o \
        ../noudar-core/noudar-core/src/CMonsterGenerator.o \
        ../noudar-core/noudar-core/src/CTeam.o \
        ../noudar-core/noudar-core/src/CMap.o \
        ../noudar-core/noudar-core/src/Common.o \
        ../noudar-core/noudar-core/src/CCharacter.o \
        ../noudar-core/noudar-core/src/CCharacterArchetype.o \
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
        ../noudar-core/noudar-core/src/commands/CSuggestItemCommand.o \
        ../noudar-core/noudar-core/src/commands/CUseCurrentItemCommand.o \
        ../noudar-core/noudar-core/src/commands/CCycleNextItemCommand.o \
        ../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.o

LIBS = $(SDL_LIB)
TARGET = x11noudar

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:   $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

packager:
	rm -f ./packer
	$(CXX) -std=c++14 -opacker packer.cpp

data:	packager
	rm -f ./data.pfs
	ls res/*.*  | xargs ./packer

demodata:	packager
	rm -f ./data.pfs
	ls resDemo/*.*  | xargs ./packer
