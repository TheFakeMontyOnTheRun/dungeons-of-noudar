#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <sys/farptr.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <time.h>
#include <unistd.h>
#include <iterator>
#include <string>
#include <memory>
#include <EASTL/vector.h>
#include <EASTL/array.h>

using eastl::vector;
using eastl::array;

#include "OPL2.h"
#include "controller.h"
#include "timer.h"

#define STR(x) #x
#define XSTR(x) STR(x)

bool timerSet = false;
int timerStart = 0;
bool enableOPL2 = false;
extern OPL2 opl2;
char buffer[3][255];
vector<uint32_t> melody;

vector<uint32_t> bgSound{};
vector<uint32_t> currentSound = bgSound;
vector<uint32_t>::const_iterator currentSoundPosition = std::begin(bgSound);
int lastFreq = -1;

short get_lpt_port(int i) {
    return _farpeekw(_dos_ds, 0x0408 + (2 * (i - 1)));
}

short setup(void) {
    cputs("OPT2LPT setup\r\n\r\n");

    char num_ports, port, i;

    num_ports = 0;
    for (i = 1; i < 4; i++) {
        if (get_lpt_port(i)) {
            num_ports++;
            port = i;
        }
    }

    if (num_ports == 0) {
        cputs("Sorry, no printer port found...\r\n");
        exit(1);
    } else if (num_ports == 1) {
        cprintf("Found one printer port: LPT%d\r\n", port);
        return get_lpt_port(port);
    } else {
        cputs("Found multiple printer ports:");
        for (i = 1; i < 4; i++) {
            if (get_lpt_port(i)) {
                cprintf(" LPT%d", i);
            }
        }
        cputs("\r\nWhich one is the OPT2LPT connected to? [");
        for (i = 1; i < 4; i++) {
            if (get_lpt_port(i)) {
                cprintf("%d", i);
            }
        }
        cputs("]? ");
        do {
            port = getch() - '0';
        } while (port < 1 || port > 3 || !get_lpt_port(port));
        cprintf("LPT%d\r\n", port);
        return get_lpt_port(port);
    }
    return 0;
}

void playMusic(const std::string &music) {
    if (enableOPL2) {
        if (music.empty()) {
            music_set("", "", "");
            return;
        }
        auto melody1 = music.substr(0, music.find(';'));
        auto melody2 = music.substr( music.find(';') + 1 );
        auto melody3 = melody2.substr( melody2.find(';') + 1 );
        melody2 = melody2.substr(0, melody2.find(';'));

        snprintf(&buffer[0][0], 254, "%s", melody1.c_str());
        snprintf(&buffer[1][0], 254, "%s", melody2.c_str());
        snprintf(&buffer[2][0], 254, "%s", melody3.c_str());
        music_set(&buffer[0][0], &buffer[1][0], &buffer[2][0]);
        return;
    } else {
            uint32_t frequency = 0;
            int octave = 0;
            melody.clear();
            int position = 0;
            for (const auto &note : music) {
                ++position;
                switch (note) {
                    case '>':
                        octave++;
                        break;
                    case '<':
                        octave--;
                        break;
                    case 'o':
                        octave = music[ position ] - '0';
                        break;
                    case 'a':
                    case 'A':
                        frequency = 28;
                        break;
                    case 'B':
                    case 'b':
                        frequency = 30;
                        break;
                    case 'C':
                    case 'c':
                        frequency = 16;
                        break;
                    case 'D':
                    case 'd':
                        frequency = 18;
                        break;
                    case 'E':
                    case 'e':
                        frequency = 20;
                        break;
                    case 'F':
                    case 'f':
                        frequency = 22;
                        break;
                    case 'G':
                    case 'g':
                        frequency = 24;
                        break;
                    case ';':
                        melody.clear();
                    default:
                        frequency = 0;
                }

                auto shifter = octave;

                if ( octave > 0 ) {
                    frequency = frequency << shifter;
                }

                melody.push_back(frequency);
            }

            if (currentSoundPosition != std::end(currentSound) && currentSound == melody) {
                return;
            }

            currentSound = melody;
            currentSoundPosition = std::begin(currentSound);
            timerStart = timer_get();
    }
}

void muteSound() {
    playMusic("");
}

void playTune(const std::string &music) {
    if ( enableOPL2 ) {
        playMusic( music + ";" + music + ";" + music );
    } else {
        playMusic( music );
    }

}

void setupOPL2(int port) {
    if ( port == -1 ) {
        short lpt_base = setup();
        opl2.init(lpt_base, false);
    } else {
        opl2.init(0x0388, true);
    }
    music_setup();
    enableOPL2 = true;
}

void soundTick() {
    if (enableOPL2) {
        music_loop();
    } else {

        if (!timerSet ) {
            timerSet = true;
            timer_setup(10);
        }

        auto timerPosition = timer_get();

        int offset = (2 * (timerPosition - timerStart)) / 3;

        if (offset < currentSound.size() && !currentSound.empty()) {

            currentSoundPosition = std::begin(currentSound) + offset;

            auto freq = *currentSoundPosition;

            if ( freq != lastFreq ) {

                sound(freq);
            }

            lastFreq = freq;

        } else {
            lastFreq = -1;
            nosound();
            currentSound = bgSound;
            currentSoundPosition = std::end(bgSound);
        }


    }
}

void stopSounds() {
    if (enableOPL2) {
        music_shutdown();
    } else {
        timer_shutdown();
        nosound();
        currentSound = bgSound;
        currentSoundPosition = std::end(bgSound);
    }
}
