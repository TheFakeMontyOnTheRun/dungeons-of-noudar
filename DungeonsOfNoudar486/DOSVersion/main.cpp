
#include <time.h>
#include <array>
#include <go32.h>
#include <sys/farptr.h>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <time.h>
#include <unistd.h>

#include <conio.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <memory>
#include <utility>
#include <string>
#include <map>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sg14/fixed_point>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;
using namespace std::chrono;
using sg14::fixed_point;

#include "NativeBitmap.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "IFileLoaderDelegate.h"
#include "CGame.h"
#include "CPlainFileLoader.h"
#include "Vec2i.h"
#include "IMapElement.h"
#include "CTeam.h"
#include "CItem.h"
#include "CActor.h"
#include "CGameDelegate.h"
#include "CMap.h"
#include "IRenderer.h"
#include "commands/IGameCommand.h"
#include "RasterizerCommon.h"
#include "CRenderer.h"
#include "CPackedFileReader.h"
#include "LoadPNG.h"

std::shared_ptr<odb::CRenderer> renderer;

enum ESoundDriver : uint8_t { kNone, kPcSpeaker, kOpl2Lpt, kTandy, kCovox };

ESoundDriver soundDriver = kNone;
int soundTiming = 0;

void initOPL2(int instrument);
void playTune(const std::string&);
void setupOPL2(int instrument);
void stopSounds();
void soundTick();
void playMusic(int instrument, const std::string &musicTrack);

void initOPL2(int instrument) {
    setupOPL2(instrument);
}


std::function< std::string(std::string)> kDosLongFileNameTransformer = [](const std::string& filename ) {
    char c = 219;
    c = 176;
    c = 177;
    c = 178;
    c = '.';
    std::cout << c;
    std::cout.flush();

    auto dotPosition = std::find( std::begin(filename), std::end( filename), '.');
    auto indexDot =  std::distance( std::begin( filename ), dotPosition );
    auto extension = filename.substr( indexDot + 1, 3 );

    if ( indexDot >  8 ) {
        return filename.substr( 0, 6 ) + "~1." + extension;
    }

    if ( filename.length() - indexDot > 4 ) {
        return filename.substr( 0, indexDot ) + "~1." + extension;
    }

    return filename;
};

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags,
                     const char* file, int line) {
    return malloc( size );
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName,
                     int flags, unsigned debugFlags, const char* file, int line) {
    return malloc( size );
}

void gameLoopTick() {
    renderer->render( 33 );
    renderer->handleSystemEvents();
}

std::shared_ptr<Knights::CGame> game;

int getch_noblock() {

    if (soundDriver != kNone ) {
        playTune("e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
    }

    while (!kbhit()) {

        if (soundDriver != kNone ) {
            usleep((75) * 1000);
            soundTick();
        }
    }

    if (soundDriver != kNone ) {
        stopSounds();
    }

    return getch();
}

int main(int argc, char **argv) {
    int instrument = -1;
    printf("Dungeons of Noudar 486 tech demo startup. Gonna load some stuff...");

    if ( argc >= 2 ) {
        //  enableSecret = true;
        if ( !std::strcmp(argv[1], "pcspeaker")) {
            soundDriver = kPcSpeaker;
            soundTiming = 100;
        }

        if ( !std::strcmp(argv[1], "opl2lpt")) {
            instrument = 80;
            soundTiming = 75;
            soundDriver = kOpl2Lpt;

            if (argc >= 3 ) {
                instrument = atoi(argv[2]);
            }

            initOPL2(instrument);
        }
    }

    const auto LEVEL_LIMIT = 7;
    auto delegate = std::make_shared<Knights::CGameDelegate>();
    auto fileLoader = std::make_shared<odb::CPackedFileReader>("data.pfs");
    auto intro = loadPNG( "intro.png", fileLoader );
    auto ready = loadPNG( "enter.png", fileLoader );

    renderer = std::make_shared<odb::CRenderer>();
    renderer->drawBitmap(0, 0, intro );
    renderer->flip();
    game = std::make_shared<Knights::CGame>( fileLoader, renderer, delegate );

    game->getMap()->getAvatar()->addHP(99);
    auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
    renderer->loadTextures( odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);

    auto onLevelLoaded = [&]() {
        if ( game->getLevelNumber() >= LEVEL_LIMIT ) {
            game->setIsPlaying( false );
        } else {
            auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
            renderer->loadTextures( odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);
        }

        renderer->drawBitmap(0, 0, intro );
        renderer->drawBitmap(0, 180, ready );
        renderer->flip();
        getch_noblock();
    };

    delegate->setOnLevelLoadedCallback(onLevelLoaded );

    auto onLevelWillLoad = [&]() {
        renderer->drawBitmap(0, 0, intro );
        renderer->flip();
    };

    delegate->setOnLevelWillLoadCallback(onLevelWillLoad );

    game->endOfTurn(game->getMap());

    renderer->drawBitmap(0, 0, intro );
    renderer->drawBitmap(0, 180, ready );
    renderer->flip();
    getch_noblock();


    auto noteTime = soundTiming;
    clock_t t0;
    clock_t t1;

    int healthAtTargetBefore = 0;
    int healthAtTargetAfter = 0;

    while ( game->isPlaying() ) {
        if (soundDriver != kNone ) {
            t0 = uclock();
        }

        auto playerHealthBefore = game->getMap()->getAvatar()->getHP();

        auto cursorPosition = game->getMap()->getTargetProjection(game->getMap()->getAvatar());
        auto actorAtTarget = game->getMap()->getActorAt(cursorPosition);

        if ( actorAtTarget != nullptr ) {
            healthAtTargetBefore = actorAtTarget->getHP();
        } else {
            healthAtTargetBefore = 0;
        }

        game->tick();

        gameLoopTick();

        Knights::CommandType command = renderer->peekInput();

        game->tick();

        if ( command != '.') {
            char buffer[81];
            snprintf(buffer, 80, "%s", game->getLastCommand().c_str());
            renderer->appendToLog( buffer );
        }


        if ( actorAtTarget != nullptr ) {
            healthAtTargetAfter = actorAtTarget->getHP();
        } else {
            healthAtTargetAfter = 0;
        }

        if ( healthAtTargetAfter < healthAtTargetBefore ) {
            char buffer[81];
            snprintf(buffer, 80, "Player dealt %d of damage", ( healthAtTargetBefore - healthAtTargetAfter ) );
            renderer->appendToLog( buffer );
            if (actorAtTarget == nullptr || !actorAtTarget->isAlive()) {
                renderer->addDeathAt(actorAtTarget->getPosition());
            } else {
                renderer->addSplatAt(actorAtTarget->getPosition());
            }

        }


        auto playerHealthAfter = game->getMap()->getAvatar()->getHP();

        auto diff = playerHealthAfter - playerHealthBefore;

        if ( diff < 0 ) {
            char buffer[81];
            snprintf(buffer, 80, "Player took %d of damage", -diff);
            renderer->appendToLog( buffer );
            renderer->startDamageHighlight();
        } else if ( diff > 0 ) {
            char buffer[81];
            snprintf(buffer, 80, "Player gained %d of faith", diff);
            renderer->appendToLog( buffer );
            renderer->startHealHighlight();
        }




        if (soundDriver != kNone ) {

            if (command == Knights::kUseCurrentItemInInventoryCommand) {
                playMusic(20, "020|aca|aca|aca");
            }

            if (command == Knights::kCycleLeftInventoryCommand) {
                playMusic(20, "020|ac|ac|ac");
            }

            if (command == Knights::kCycleRightInventoryCommand) {
                playMusic(20, "020|ca|ca|ca");
            }

            if (command == Knights::kPickItemCommand) {
                playMusic(20, "020|abc|abc|abc");
            }

            if (command == Knights::kDropItemCommand) {
                playMusic(20, "020|cba|cba|cba");
            }

            t1 = uclock();
            auto diff = (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
            if (diff == 0) {
                diff = 1;
            }
            noteTime -= diff;

            if (noteTime < 0) {
                noteTime = soundTiming;
                soundTick();
            }
        }

    }
    stopSounds();

    return 0;
}
