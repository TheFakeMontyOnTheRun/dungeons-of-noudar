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
#include <vector>
#include <array>
#include <memory>

#include "OPL2.h"
#include "controller.h"

#define STR(x) #x
#define XSTR(x) STR(x)

bool enableOPL2 = false;
extern OPL2 opl2;
std::vector<int> noSound;
std::vector<int> melody;
std::vector<int>::const_iterator currentSoundPosition;
int currentNote = 0;
std::string buffer[3];

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


void playSound(const std::vector<int> &sound) {

    if ( sound.empty() ) {
        melody.clear();
        return;
    }

    melody = sound;
    currentSoundPosition = std::begin(melody);
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
        buffer[0] = melody1;
        buffer[1] = melody2;
        buffer[2] = melody3;
        music_set(buffer[0].c_str(), buffer[1].c_str(), buffer[2].c_str());
        return;
    }

    int frequency = 0;
    melody.clear();
    for (const auto &note : music) {
        switch (note) {
            case 'a':
            case 'A':
                frequency = 932;
                break;
            case 'B':
            case 'b':
                frequency = 988;
                break;
            case 'C':
            case 'c':
                frequency = 1109;
                break;
            case 'D':
            case 'd':
                frequency = 1175;
                break;
            case 'E':
            case 'e':
                frequency = 1318;
                break;
            case 'F':
            case 'f':
                frequency = 1397;
                break;
            case 'G':
            case 'g':
                frequency = 1568;
                break;
            case ';':
                melody.clear();
                continue;
        }
        melody.push_back(frequency);
    }
    playSound(melody);
}

void muteSound() {
    if ( !enableOPL2 ) {
        nosound();
        melody.clear();
        currentNote = 0;
    } else {
        playMusic("");
    }
}

void playTune(const std::string &music) {
    buffer[0] = music + ";" + music + ";" + music;
    hackTune( music.c_str() );
}

void setupOPL2() {
    short lpt_base = setup();
    opl2.init(lpt_base);
    music_setup();
    enableOPL2 = true;
}

void soundFrequency(int frequency) {
    if (!enableOPL2) {
        if (frequency != 0 && currentNote != frequency ) {
            sound( frequency );
            currentNote = frequency;
        }
    }
}


void soundTick() {
    if (enableOPL2) {
        music_loop();
    } else {
        if (melody.empty()) {
            return;
        }

        if (currentSoundPosition != std::end(melody)) {
            soundFrequency(*currentSoundPosition);
            currentSoundPosition = std::next(currentSoundPosition);
        } else {
            muteSound();
            melody.clear();
        }
    }
}

void stopSounds() {
    if (enableOPL2) {
        music_shutdown();
    } else {
        muteSound();
    }
}
