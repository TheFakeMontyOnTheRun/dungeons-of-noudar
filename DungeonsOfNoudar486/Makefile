CXX = i386-pc-msdosdjgpp-g++
LD = i386-pc-msdosdjgpp-ld

CXXFLAGS = -g -DVGA -DDONT_LOAD_LEVEL_UPON_START -Iinclude -c -Istb -fpermissive -c -O6 -Ofast  -funroll-loops -funroll-all-loops -fomit-frame-pointer -fno-exceptions -ffast-math -mtune=i486 -march=i486 -I../fixed_point/include -IDOS-version -Iinclude -I../stb -I../noudar-core/noudar-core/include   -I../noudar-rendering -I../noudar-rendering -fpermissive -Wno-narrowing -DUSE_ITEMS_INSTANTLY

LDFLAGS = -lemu

OBJS = src/CRenderer.o \
    src/CRenderer_Rasterization.o \
    src/CRenderer_Tesselation.o \
     src/RasterizerCommon.o \
     src/NativeBitmap.o \
     src/CTile3DProperties.o \
     src/CPackedFileReader.o \
     src/VisibilityStrategy.o \
     OPL2LPT/OPL2.o \
     OPL2LPT/timer.o \
     OPL2LPT/controller.o \
     OPL2LPT/MusicDOS.o \
     ../noudar-rendering/Logger.o \
    ../noudar-rendering/LoadPNG.o \
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
    ../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.o \
    main.o \
    events.o \
    DOSVersion/CDOSRenderer.o \

TARGET = noudar.exe
DATA = data.pfs

$(TARGET):	$(OBJS)  $(DATA)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) $(LDFLAGS)
	i386-pc-msdosdjgpp-strip $(TARGET)

all:   $(TARGET)

packager:
	rm -f ./packer
	g++ -std=c++14 -opacker packer.cpp
	

$(DATA): packager
	rm -f ./data.pfs
	ls res/*.*  | xargs ./packer

clean:
	rm -f $(OBJS) $(TARGET)
