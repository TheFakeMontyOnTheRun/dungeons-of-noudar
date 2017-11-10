CXX = clang++

SDL_LIB = `sdl-config --libs` -lSDL_gfx -lSDL_image -lSDL_mixer
SDL_INCLUDE =  `sdl-config --cflags `

CXXFLAGS = -std=c++14 -O2 -g $(SDL_INCLUDE) -Iinclude -Ifixed_point/include -Istb -Icore/noudar-core/include  -Icore/EASTL/include -Icore/EASTL/test/packages/EABase/include/Common 

OBJS = SDLVersion/main.o \
        SDLVersion/CSDLRenderer.o \
        src/CRenderer.o \
        src/RaycastCommon.o \
        src/NativeBitmap.o \
        src/LoadPNG.o \
        core/noudar-core/src/Vec2i.o \
        core/noudar-core/src/CActor.o \
        core/noudar-core/src/CMonster.o \
        core/noudar-core/src/CMonsterGenerator.o \
        core/noudar-core/src/CTeam.o \
        core/noudar-core/src/CMap.o \
        core/noudar-core/src/Common.o \
        core/noudar-core/src/CCharacter.o \
        core/noudar-core/src/CCharacterArchetype.o \
        core/noudar-core/src/CDoorway.o \
        core/noudar-core/src/IFileLoaderDelegate.o \
        core/noudar-core/src/IMapElement.o \
        core/noudar-core/src/CGame.o \
        core/noudar-core/src/CItem.o \
        core/noudar-core/src/CStorageItem.o \
        core/noudar-core/src/CPlainFileLoader.o \
        core/noudar-core/src/CGameDelegate.o \
        core/noudar-core/src/commands/IGameCommand.o \
        core/noudar-core/src/commands/CMoveActorCommand.o \
        core/noudar-core/src/commands/CLoadNewLevelCommand.o \
        core/noudar-core/src/commands/CTurnActorCommand.o \
        core/noudar-core/src/commands/CQuitCommand.o \
        core/noudar-core/src/commands/CNullCommand.o \
        core/noudar-core/src/commands/CEndTurnCommand.o \
        core/noudar-core/src/commands/CPickItemCommand.o \
        core/noudar-core/src/commands/CDropItemCommand.o \
        core/noudar-core/src/commands/CUseCurrentItemCommand.o \
        core/noudar-core/src/commands/CCycleNextItemCommand.o \
        core/noudar-core/src/commands/CCyclePreviousItemCommand.o

LIBS = $(SDL_LIB)
TARGET = game

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:   $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
