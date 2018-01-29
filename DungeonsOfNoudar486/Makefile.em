CXXFLAGS = -O3 -Wall -DEA_PLATFORM_SAMSUNG_TV  -DSDLSW -DVGA  -g  -IDOS-version -I../stb -I../fixed_point/include -fpermissive -Wno-narrowing  -c -std=c++14 -ferror-limit=1  -s USE_ZLIB=1 -s USE_LIBPNG=1 -I../noudar-core/noudar-core/include -Iinclude -I../noudar-rendering -I../gles2-renderer/ -s ALLOW_MEMORY_GROWTH=1 -I../noudar-core/EASTL/include -I../noudar-core/EASTL/test/packages/EABase/include/Common -DUSE_ITEMS_INSTANTLY  -fpermissive -Wno-narrowing
LDFLAGS =  -O3 -s USE_ZLIB=1 -s USE_LIBPNG=1 -s --preload-file emdata --use-preload-plugins  -s ALLOW_MEMORY_GROWTH=1
CXX = em++
CC = emcc

#CXXFLAGS = -Wall -std=c++14 -O3 --preload-file emdata --use-preload-plugins  -g -DSDLSW -DVGA -DEA_PLATFORM_SAMSUNG_TV -Iinclude -c -Istb -fpermissive -fomit-frame-pointer -fno-exceptions -ffast-math -I../fixed_point/include -IDOS-version -I../fixed_point/include -Iinclude -I../stb -I../noudar-core/noudar-core/include  -I../noudar-core/EASTL/include -I../noudar-core/EASTL/test/packages/EABase/include/Common -I../noudar-rendering -I../gles2-renderer -fpermissive -Wno-narrowing -DUSE_ITEMS_INSTANTLY -s DEMANGLE_SUPPORT=1 -s ALLOW_MEMORY_GROWTH=1 -Wno-narrowing
#LDFLAGS = -O3 --preload-file emdata --use-preload-plugins  -s USE_ZLIB=1 -s USE_LIBPNG=1 -s -s DEMANGLE_SUPPORT=1 -s ALLOW_MEMORY_GROWTH=1 ASSERTIONS=2

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
        ../noudar-core/noudar-core/src/commands/CUseCurrentItemCommand.o \
        ../noudar-core/noudar-core/src/commands/CCycleNextItemCommand.o \
        ../noudar-core/noudar-core/src/commands/CCyclePreviousItemCommand.o

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

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

