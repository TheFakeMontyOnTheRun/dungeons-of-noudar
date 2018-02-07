#if defined(WINVER)
#include <windows.h>
#endif

#ifdef __DJGPP__
#else
const long UCLOCKS_PER_SEC = 1000;

long uclock();

#endif


//general stuff below
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

enum ESoundDriver : uint8_t {
    kNone, kPcSpeaker, kOpl2Lpt, kTandy, kCovox
};

ESoundDriver soundDriver = kNone;

void initOPL2();

void playTune(const std::string &);

void setupOPL2();

void stopSounds();

void soundTick();

void muteSound();

void playMusic(const std::string &musicTrack);

namespace odb {
    std::shared_ptr<CRenderer> renderer = nullptr;
}


void initOPL2() {
    setupOPL2();
}

void *operator new[](size_t size, const char *pName, int flags, unsigned debugFlags,
                     const char *file, int line) {
    return new uint8_t[size];
}

void *operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char *pName,
                     int flags, unsigned debugFlags, const char *file, int line) {
    return new uint8_t[size];;
}

void renderTick(long ms) {
    odb::renderer->render(ms);
    odb::renderer->handleSystemEvents();
}

std::shared_ptr<Knights::CGame> game;

int getchWithSoundTicks() {
    int keycode = 0;

    if (soundDriver != kNone) {
        while (true) {
            soundTick();

            if (odb::peekKeyboard(odb::renderer)) {
                keycode = odb::readKeyboard(odb::renderer);

                if (keycode == 13) {
                    muteSound();
                    return keycode;
                }
            }
        }
    } else {
        do {
            keycode = odb::readKeyboard(odb::renderer);
        } while (keycode != 13);
    }

    return keycode;
}


void showText(std::shared_ptr<odb::NativeBitmap> bg, const std::string &mainText, const std::string &bottom,
              uint8_t bgSolidColour = 0, uint8_t startingLine = 9) {
    odb::renderer->fill(0, 0, 320, 200, bgSolidColour);
    odb::renderer->drawBitmap(0, 0, bg);

    uint8_t mainTextColour = odb::renderer->getPaletteEntry(0xFFFF0000);
    uint8_t bottomTextColour = odb::renderer->getPaletteEntry(0xFF888888);
    odb::renderer->drawTextAt(1, startingLine, mainText.c_str(), mainTextColour);
    odb::renderer->drawTextAt(1, 25, bottom.c_str(), bottomTextColour);
    odb::renderer->flip();
}

void handleConsoleLines(Knights::CommandType command, int playerHealthDiff, int targetHealthDiff,
                        std::shared_ptr<odb::CRenderer> renderer, std::shared_ptr<Knights::CActor> actorAtTarget) {

    uint8_t playerDealtDamageColour = renderer->getPaletteEntry(0xFFFF0000);
    uint8_t playerTookDamageColour = renderer->getPaletteEntry(0xFF0000FF);
    uint8_t playerGainedFaithColour = renderer->getPaletteEntry(0xFF00FF00);
    uint8_t commandColour = renderer->getPaletteEntry(0xFFFFFFFF);
    uint8_t effectColour = renderer->getPaletteEntry(0xFF888888);

    char *soundToPlay = nullptr;

    if (command != '.') {
        char buffer[41];
        snprintf(&buffer[0], 39, "%s", game->getLastCommand().c_str());
        renderer->appendToLog(buffer, commandColour);

        auto say = game->getMap()->getAvatar()->getCurrentSay();
        if (!say.empty()) {
            snprintf(&buffer[0], 39, "%s", say.c_str());
            renderer->appendToLog(buffer, effectColour);
            game->getMap()->getAvatar()->setCurrentSay("");
        }
    }


    if (command == Knights::kUseCurrentItemInInventoryCommand) {
        playTune("i114t1o8f");
    }

    if (command == Knights::kCycleRightInventoryCommand) {
        playTune("t200i98a");
    }

    if (command == Knights::kPickItemCommand) {
        playTune("t200i9o1fa");
    }

    if (targetHealthDiff < 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player dealt %d on %s", targetHealthDiff, actorAtTarget->getName().c_str());
        renderer->appendToLog(buffer, playerDealtDamageColour);
        if (actorAtTarget == nullptr || !actorAtTarget->isAlive()) {
            renderer->addDeathAt(actorAtTarget->getPosition());
            playTune("t200i101o8ao4ao2ao1a");
        } else {
            renderer->addSplatAt(actorAtTarget->getPosition());
            playTune("t200i101o3afo1a");
        }
    }

    if (playerHealthDiff < 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player took %d of damage", std::abs(playerHealthDiff));
        renderer->appendToLog(buffer, playerTookDamageColour);
        renderer->startDamageHighlight();
        playTune("t200i53o3fo1f");
    } else if (playerHealthDiff > 0) {
        char buffer[41];
        snprintf(&buffer[0], 39, "Player gained %d of faith", std::abs(playerHealthDiff));
        renderer->appendToLog(buffer, playerGainedFaithColour);
        renderer->startHealHighlight();
        playTune("t200i52o4defg");
    }
}

#ifdef __APPLE__
extern "C" int SDL_main(int argc, char **argv) {
#else

#if defined(WINVER)

namespace odb {
    LRESULT CALLBACK WindProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
#else

int main(int argc, char **argv) {
    puts("Dungeons of Noudar 3D startup. Gonna load some stuff...");

    if (argc >= 2) {
        if (!std::strcmp(argv[1], "opl2lpt")) {
            soundDriver = kOpl2Lpt;
            initOPL2();
            playTune("t200i101o1a");
        }
    }

#endif

#endif

    int instrument = -1;

    const auto LEVEL_LIMIT = 7;
    clock_t t0;
    clock_t t1;
    int healthAtTargetBefore = 0;
    int healthAtTargetAfter = 0;
    auto delegate = std::make_shared<Knights::CGameDelegate>();
    auto fileLoader = std::make_shared<odb::CPackedFileReader>("data.pfs");
    auto bg = loadPNG("intro.png", fileLoader);


    odb::renderer = std::make_shared<odb::CRenderer>(fileLoader);

    auto titleText = fileLoader->loadFileFromPath("title.txt");
    {
        auto logo = loadPNG("logo.png", fileLoader);
        showText(logo, titleText, "Press Enter to continue");
    }
    getchWithSoundTicks();

    auto onLevelWillLoad = [&]() {
        showText(bg, "", "Loading...");
    };
    delegate->setOnLevelWillLoadCallback(onLevelWillLoad);

    game = std::make_shared<Knights::CGame>(fileLoader, odb::renderer, delegate);
    auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
    odb::renderer->loadTextures(odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader), tileProperties);

    char buffer[41];
    snprintf(buffer, 39, "chapter0.txt");
    auto introText = fileLoader->loadFileFromPath(buffer);

    if (soundDriver != kNone) {
        //    playTune("t120e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
    }

    showText(bg, introText, "Press Enter to start");
    getchWithSoundTicks();

    auto onLevelLoaded = [&]() {

        if (game != nullptr) {
            if (game->getLevelNumber() >= LEVEL_LIMIT) {
                game->setIsPlaying(false);


                game->setIsPlaying(false);

                bg = loadPNG("finis_gloriae_mundi.png", fileLoader);

                if (soundDriver != kNone) {
                    //              playTune("t120e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
                }

                showText(bg, fileLoader->loadFileFromPath("chapter7.txt"), "                    Press enter to exit",
                         odb::renderer->getPaletteEntry(0xFFFFFFFF), 2);
                getchWithSoundTicks();
                return;
            } else {
                auto tileProperties = odb::loadTileProperties(game->getLevelNumber(), fileLoader);
                odb::renderer->loadTextures(odb::loadTexturesForLevel(game->getLevelNumber(), fileLoader),
                                            tileProperties);
            }

            char buffer[41];
            snprintf(buffer, 39, "chapter%d.txt", game->getLevelNumber());
            auto chapterText = fileLoader->loadFileFromPath(buffer);

            if (soundDriver != kNone) {
                //        playTune("t120e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
            }

            showText(bg, chapterText, "Press Enter to continue");
            getchWithSoundTicks();
        }
    };
    delegate->setOnLevelLoadedCallback(onLevelLoaded);

    clock_t diff = 0;
    while (game->isPlaying()) {
        t0 = uclock();

        auto playerHealthBefore = game->getMap()->getAvatar()->getHP();
        auto cursorPosition = game->getMap()->getTargetProjection(game->getMap()->getAvatar());
        auto actorAtTarget = game->getMap()->getActorAt(cursorPosition);

        if (actorAtTarget != nullptr) {
            healthAtTargetBefore = actorAtTarget->getHP();
        } else {
            healthAtTargetBefore = 0;
        }

        game->tick();
        renderTick(diff);
        Knights::CommandType command = odb::renderer->peekInput();
        game->tick();
        soundTick();

        if (actorAtTarget != nullptr) {
            healthAtTargetAfter = actorAtTarget->getHP();
        } else {
            healthAtTargetAfter = 0;
        }

        auto targetHealthDiff = healthAtTargetAfter - healthAtTargetBefore;
        auto playerHealthAfter = game->getMap()->getAvatar()->getHP();
        auto playerHealthDiff = playerHealthAfter - playerHealthBefore;

        handleConsoleLines(command, playerHealthDiff, targetHealthDiff, odb::renderer, actorAtTarget);

        t1 = uclock();
        diff = (1000 * (t1 - t0)) / UCLOCKS_PER_SEC;
        if (diff == 0) {
            diff = 1;
        }

        if (!game->getMap()->getAvatar()->isAlive()) {
            game->setIsPlaying(false);

            bg = loadPNG("in_ictu_oculi.png", fileLoader);

            if (soundDriver != kNone) {
                //      playTune("t120e8e8f8g8g8f8e8d8c8c8d8e8e8d12d4e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4d8d8e8c8d8e12f12e8c8d8e12f12e8d8c8d8p8e8e8f8g8g8f8e8d8c8c8d8e8d8c12c4");
            }

            showText(bg, fileLoader->loadFileFromPath("gameover.txt"), "                    Press enter to exit",
                     odb::renderer->getPaletteEntry(0xFFFFFFFF), 2);
            getchWithSoundTicks();
        }

    }

    if (soundDriver != kNone) {
        stopSounds();
    }

    return 0;
}
