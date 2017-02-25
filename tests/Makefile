GTEST_DIR = ../googletest/googletest
GMOCK_DIR = ../googletest/googlemock

CXXFLAGS = -I../noudar-core/noudar-core/include -I../glm -I../gles2-renderer -I../noudar-rendering \
		-I$(GTEST_DIR)/include \
		-I$(GTEST_DIR) \
		-I$(GTEST_DIR)/include/gtest \
		-I$(GTEST_DIR)/include/gtest/internal \
		-I$(GMOCK_DIR)/include \
		-I$(GMOCK_DIR) \
		-I$(GMOCK_DIR)/include/gtest \
		-I$(GMOCK_DIR)/include/gtest/internal \
		-c -std=c++0x -g -O0 -Wall
CXX = clang++

OBJS =  ../noudar-core/noudar-core/src/CActor.o \
	../noudar-core/noudar-core/src/Vec2i.o \
	../noudar-core/noudar-core/src/CTeam.o \
	../noudar-core/noudar-core/src/CCharacter.o \
	../noudar-core/noudar-core/src/CCharacterArchetype.o \
	../noudar-core/noudar-core/src/CMonster.o \
	../noudar-core/noudar-core/src/CMap.o \
	../noudar-core/noudar-core/src/CItem.o \
	../noudar-core/noudar-core/src/Common.o \
	../noudar-core/noudar-core/src/CHuntApproach.o \
	../noudar-core/noudar-core/src/CElixirFountain.o \
	../noudar-core/noudar-core/src/CDoorway.o \
	../noudar-core/noudar-core/src/IFileLoaderDelegate.o \
	../noudar-core/noudar-core/src/IMapElement.o \
	../noudar-core/noudar-core/src/CGame.o \
	../noudar-core/noudar-core/src/CPlainFileLoader.o \
	../noudar-core/noudar-core/src/CGameDelegate.o \
	../noudar-core/noudar-core/src/commands/CActorMeleeAttackCommand.o \
	../noudar-core/noudar-core/src/commands/IGameCommand.o \
	../noudar-core/noudar-core/src/commands/CMoveActorCommand.o \
	../noudar-core/noudar-core/src/commands/CHitscanAttackCommand.o \
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
	../gles2-renderer/VBORenderingJob.o \
	$(GTEST_DIR)/src/gtest-all.o \
	$(GMOCK_DIR)/src/gmock-all.o \
	$(GMOCK_DIR)/src/gmock_main.o \
	src/TestCCharacter.o 

#	src/TestRenderingJobSnapshotAdapter.o

TARGET = test
LDFLAGS = -lpthread

$(TARGET): $(OBJS) 
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	rm src/*~
	rm include/*~
	rm *~
