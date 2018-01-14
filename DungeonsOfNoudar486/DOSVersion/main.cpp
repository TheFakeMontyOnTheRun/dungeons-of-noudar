
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

void renderTick() {
    renderer->render( 33 );
    renderer->handleSystemEvents();
}

std::shared_ptr<Knights::CGame> game;

int getchWithSoundTicks() {

    if (soundDriver != kNone ) {
        while (!kbhit()) {
            usleep((75) * 1000);
            soundTick();
        }

        stopSounds();
    }

    return getch();
}


void showText(std::shared_ptr<odb::NativeBitmap> bg, const std::string& mainText, const std::string& bottom ) {
    renderer->drawBitmap(0, 0, bg );
    renderer->fill(0, 64, 320, 200 - 64, 0 );
    renderer->flip();
    gotoxy(1,9);
    puts(mainText.c_str());
    gotoxy(1,25);
    printf(bottom.c_str());
}

void playSoundForAction(Knights::CommandType command ) {
    if (command == Knights::kUseCurrentItemInInventoryCommand) {
        playTune("aca");
    }

    if (command == Knights::kCycleLeftInventoryCommand) {
        playTune("ac");
    }

    if (command == Knights::kCycleRightInventoryCommand) {
        playTune("ca");
    }

    if (command == Knights::kPickItemCommand) {
        playTune("abc");
    }

    if (command == Knights::kDropItemCommand) {
        playTune("cba");
    }
}

int main(int argc, char **argv) {
    int instrument = -1;

    const auto LEVEL_LIMIT = 7;
    clock_t t0;
    clock_t t1;
    int healthAtTargetBefore = 0;
    int healthAtTargetAfter = 0;
    auto delegate = std::make_shared<Knights::CGameDelegate>();
    auto fileLoader = std::make_shared<odb::CPackedFileReader>("data.pfs");
    auto bg = loadPNG( "intro.png", fileLoader );
    auto introText = fileLoader->loadFileFromPath("intro.txt");

    puts("Dungeons of Noudar 486 tech demo startup. Gonna load some stuff...");

    if ( argc >= 2 ) {
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

    renderer = std::make_shared<odb::CRenderer>();

    auto onLevelWillLoad = [&]() {
        showText(bg, "", "Loading...");
    };
    delegate->setOnLevelWillLoadCallback(onLevelWillLoad );

    game = std::make_shared<Knights::CGame>( fileLoader, renderer, delegate );
    auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
    renderer->loadTextures( odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);

    playTune("e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
    showText(bg, introText, "Press any key to start" );
    getchWithSoundTicks();

    auto onLevelLoaded = [&]() {

        if (game != nullptr ) {
            if ( game->getLevelNumber() >= LEVEL_LIMIT ) {
                game->setIsPlaying( false );
            } else {
                auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
                renderer->loadTextures( odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);
            }
        }

        playTune("e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
        showText(bg, "", "Press any key to start");
        getchWithSoundTicks();
    };
    delegate->setOnLevelLoadedCallback(onLevelLoaded );

    auto noteTime = soundTiming;

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
        renderTick();
        Knights::CommandType command = renderer->peekInput();
        game->tick();

        if ( actorAtTarget != nullptr ) {
            healthAtTargetAfter = actorAtTarget->getHP();
        } else {
            healthAtTargetAfter = 0;
        }

        if ( command != '.') {
            char buffer[81];
            snprintf(buffer, 80, "%s", game->getLastCommand().c_str());
            renderer->appendToLog( buffer );
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

            playSoundForAction(command);

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
