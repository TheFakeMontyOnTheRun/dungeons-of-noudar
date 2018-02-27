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

#define STR(x) #x
#define XSTR(x) STR(x)

bool enableOPL2 = false;
extern OPL2 opl2;
char buffer[3][255];

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
    }
}

void muteSound() {
    playMusic("");
}

void playTune(const std::string &music) {
    playMusic( music + ";" + music + ";" + music );
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
    }
}

void stopSounds() {
    if (enableOPL2) {
        music_shutdown();
    }
}
