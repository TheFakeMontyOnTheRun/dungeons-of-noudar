
#include <time.h>
#include <array>
#include <time.h>
#include <unistd.h>

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

const long UCLOCKS_PER_SEC = 1000;

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

long uclock();
int getch();
bool kbhit();
void initOPL2();
void playTune(const std::string&);
void setupOPL2();
void stopSounds();
void soundTick();
void playMusic(int instrument, const std::string &musicTrack);

void initOPL2() {
    setupOPL2();
}

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags,
                     const char* file, int line) {
    return malloc( size );
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName,
                     int flags, unsigned debugFlags, const char* file, int line) {
    return malloc( size );
}

void renderTick(long diff) {
    renderer->render( diff );
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
    renderer->drawTextAt(1, 9, mainText.c_str());
    renderer->drawTextAt(1,25, bottom.c_str()  );
}

void playSoundForAction(Knights::CommandType command ) {
    if (command == Knights::kUseCurrentItemInInventoryCommand) {
        playTune("t240a9c9a9");
    }

    if (command == Knights::kCycleLeftInventoryCommand) {
        playTune("t240a9b9c9");
    }

    if (command == Knights::kCycleRightInventoryCommand) {
        playTune("t240c9b9a9");
    }

    if (command == Knights::kPickItemCommand) {
        playTune("t240d9e9f9g9");
    }

    if (command == Knights::kDropItemCommand) {
        playTune("t240g9f9e9d9");
    }
}

void handleConsoleLines( Knights::CommandType command, int playerHealthDiff, int targetHealthDiff, std::shared_ptr<odb::CRenderer> renderer, std::shared_ptr<Knights::CActor> actorAtTarget ) {
    if ( command != '.') {
        char buffer[41];
        snprintf(buffer, 39, "%s", game->getLastCommand().c_str());
        renderer->appendToLog( buffer );
    }

    if ( targetHealthDiff < 0 ) {
        char buffer[41];
        snprintf(buffer, 39, "Player dealt %d of damage", -targetHealthDiff );
        renderer->appendToLog( buffer );
        if (actorAtTarget == nullptr || !actorAtTarget->isAlive()) {
            renderer->addDeathAt(actorAtTarget->getPosition());
        } else {
            renderer->addSplatAt(actorAtTarget->getPosition());
        }
    }

    if ( playerHealthDiff < 0 ) {
        char buffer[41];
        snprintf(buffer, 39, "Player took %d of damage", -playerHealthDiff);
        renderer->appendToLog( buffer );
        renderer->startDamageHighlight();
    } else if ( playerHealthDiff > 0 ) {
        char buffer[41];
        snprintf(buffer, 39, "Player gained %d of faith", playerHealthDiff);
        renderer->appendToLog( buffer );
        renderer->startHealHighlight();
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

    puts("Dungeons of Noudar 486 tech demo startup. Gonna load some stuff...");

    if ( argc >= 2 ) {
        if ( !std::strcmp(argv[1], "pcspeaker")) {
            soundDriver = kPcSpeaker;
            soundTiming = 100;
        }

        if ( !std::strcmp(argv[1], "opl2lpt")) {
            soundTiming = 75;
            soundDriver = kOpl2Lpt;
            initOPL2();
        }
    }

    renderer = std::make_shared<odb::CRenderer>();

    auto titleText = fileLoader->loadFileFromPath("title.txt");
    showText(bg, titleText, "Press any key to continue");
    getchWithSoundTicks();

    auto onLevelWillLoad = [&]() {
        showText(bg, "", "Loading...");
    };
    delegate->setOnLevelWillLoadCallback(onLevelWillLoad );

    game = std::make_shared<Knights::CGame>( fileLoader, renderer, delegate );
    auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
    renderer->loadTextures( odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);

    char buffer[41];
    snprintf(buffer, 39, "chapter0.txt", game->getLevelNumber() );
    auto introText = fileLoader->loadFileFromPath(buffer);

    if (soundDriver != kNone ) {
        playTune("t120e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
    }

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

            char buffer[41];
            snprintf(buffer, 39, "chapter%d.txt", game->getLevelNumber() );
            auto chapterText = fileLoader->loadFileFromPath(buffer);

            if (soundDriver != kNone ) {
                playTune("t120e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
            }

            showText(bg, chapterText, "Press any key to continue");
            getchWithSoundTicks();
        }
    };
    delegate->setOnLevelLoadedCallback(onLevelLoaded );

    auto noteTime = soundTiming;

    clock_t diff = 0;
    while ( game->isPlaying() ) {
        t0 = uclock();

        auto playerHealthBefore = game->getMap()->getAvatar()->getHP();
        auto cursorPosition = game->getMap()->getTargetProjection(game->getMap()->getAvatar());
        auto actorAtTarget = game->getMap()->getActorAt(cursorPosition);

        if ( actorAtTarget != nullptr ) {
            healthAtTargetBefore = actorAtTarget->getHP();
        } else {
            healthAtTargetBefore = 0;
        }

        game->tick();
        renderTick(diff);
        Knights::CommandType command = renderer->peekInput();
        game->tick();

        if ( actorAtTarget != nullptr ) {
            healthAtTargetAfter = actorAtTarget->getHP();
        } else {
            healthAtTargetAfter = 0;
        }

        auto targetHealthDiff = healthAtTargetAfter - healthAtTargetBefore;
        auto playerHealthAfter = game->getMap()->getAvatar()->getHP();
        auto playerHealthDiff = playerHealthAfter - playerHealthBefore;

        handleConsoleLines( command, playerHealthDiff, targetHealthDiff, renderer, actorAtTarget );

        if (soundDriver != kNone ) {
            playSoundForAction(command);
            soundTick();
        }

        t1 = uclock();
        diff = (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
        if (diff == 0) {
            diff = 1;
        }



        if ( !game->getMap()->getAvatar()->isAlive()) {
            game->setIsPlaying(false);
            auto chapterText = fileLoader->loadFileFromPath("gameover.txt");

            if (soundDriver != kNone ) {
                playTune("e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
            }

            showText(bg, chapterText, "Press any key to continue");
            getchWithSoundTicks();
        }

    }

    stopSounds();

    return 0;
}
