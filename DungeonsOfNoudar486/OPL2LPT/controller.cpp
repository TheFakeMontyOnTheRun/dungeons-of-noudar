/**
 * This is a demonstration sketch for the OPL2 Audio Board. It demonstrates playing a little tune on 3 channels using
 * a piano from the MIDI instrument defenitions.
 *
 * Code by Maarten Janssen (maarten@cheerful.nl) 2016-04-13
 * Most recent version of the library can be found at my GitHub: https://github.com/DhrBaksteen/ArduinoOPL2
 */

/**
 * Hacked for a OPL2PRN test program by pdewacht@gmail.com.
 */
#include <cstring>
#include <algorithm>


#include "OPL2.h"
#include "midi_instruments.h"
#include "timer.h"
#include "controller.h"

char* instruments[] = {
        PIANO1, //0
        PIANO2,
        PIANO3,
        HONKTONK,
        EP1,
        EP2,
        HARPSIC,
        CLAVIC,
        CELESTA,
        GLOCK,
        MUSICBOX,
        VIBES,
        MARIMBA,
        XYLO,
        TUBEBELL,
        SANTUR,
        ORGAN1,
        ORGAN2,
        ORGAN3,
        PIPEORG,
        REEDORG,
        ACORDIAN,
        HARMONIC,
        BANDNEON,
        NYLONGT,
        STEELGT,
        JAZZGT,
        CLEANGT,
        MUTEGT,
        OVERDGT,
        DISTGT,
        GTHARMS,
        ACOUBASS,
        FINGBASS,
        PICKBASS,
        FRETLESS,
        SLAPBAS1,
        SLAPBAS2,
        SYNBASS1,
        SYNBASS2,
        VIOLIN,
        VIOLA,
        CELLO,
        CONTRAB,
        TREMSTR,
        PIZZ,
        HARP,
        TIMPANI,
        STRINGS,
        SLOWSTR,
        SYNSTR1,
        SYNSTR2,
        CHOIR,
        OOHS,
        SYNVOX,
        ORCHIT,
        TRUMPET,
        TROMBONE,
        TUBA,
        MUTETRP,
        FRHORN,
        BRASS1,
        SYNBRAS1,
        SYNBRAS2,
        SOPSAX,
        ALTOSAX,
        TENSAX,
        BARISAX,
        OBOE,
        ENGLHORN,
        BASSOON,
        CLARINET,
        PICCOLO,
        FLUTE1,
        RECORDER,
        PANFLUTE,
        BOTTLEB,
        SHAKU,
        WHISTLE,
        OCARINA,
        SQUARWAV,
        SAWWAV,
        SYNCALLI,
        CHIFLEAD,
        CHARANG,
        SOLOVOX,
        FIFTHSAW,
        BASSLEAD,
        FANTASIA,
        WARMPAD,
        POLYSYN,
        SPACEVOX,
        BOWEDGLS,
        METALPAD,
        HALOPAD,
        SWEEPPAD,
        ICERAIN,
        SOUNDTRK,
        CRYSTAL,
        ATMOSPH,
        BRIGHT,
        GOBLIN,
        ECHODROP,
        STARTHEM,
        SITAR,
        BANJO,
        SHAMISEN,
        KOTO,
        KALIMBA,
        BAGPIPE,
        FIDDLE,
        SHANNAI,
        TINKLBEL,
        AGOGO,
        STEELDRM,
        WOODBLOK,
        TAIKO,
        MELOTOM,
        SYNDRUM,
        REVRSCYM,
        FRETNOIS,
        BRTHNOIS,
        SEASHORE,
        BIRDS,
        TELEPHON,
        HELICOPT,
        APPLAUSE
};

void parseTune(struct Tune *tune);

void parseNote(struct Tune *tune);

int parseDuration(struct Tune *tune);

int parseNumber(struct Tune *tune);

const byte noteDefs[21] = {
        NOTE_A, NOTE_A - 1, NOTE_A + 1,
        NOTE_B, NOTE_B - 1, NOTE_B + 1,
        NOTE_C, NOTE_C - 1, NOTE_C + 1,
        NOTE_D, NOTE_D - 1, NOTE_D + 1,
        NOTE_E, NOTE_E - 1, NOTE_E + 1,
        NOTE_F, NOTE_F - 1, NOTE_F + 1,
        NOTE_G, NOTE_G - 1, NOTE_G + 1
};

unsigned tempo;

struct Tune {
    const char *data = "";
    int channel = 0;
    int octave = 0;
    int noteDuration = 0;
    char noteLength = 0;
    unsigned long nextNoteTime = 0;
    unsigned long releaseTime = 0;
    int index = 0;
    int instrument = 0;
};

const char *tuneData[3] = {
        "",
        "",
        ""
};

OPL2 opl2;
struct Tune music[3];

void music_set(const char* melody1, const char* melody2, const char* melody3) {
    tuneData[0] = melody1;
    tuneData[1] = melody2;
    tuneData[2] = melody3;
    // Initialize 3 channels of the tune.
    for (int i = 0; i < 3; i++) {
        struct Tune channel;
        channel.data = tuneData[i];
        channel.channel = i;
        channel.octave = 4;
        channel.noteDuration = 50;
        channel.noteLength = 50;
        channel.releaseTime = 0;
        channel.nextNoteTime = timer_get();
        channel.index = 0;
        channel.instrument = 0;
        music[i] = channel;
    }

    // Setup channels 0, 1 and 2.
    opl2.setInstrument(0, instruments[ 0 ]);
    opl2.setBlock(0, 5);
    opl2.setInstrument(1, instruments[ 0 ]);
    opl2.setBlock(1, 4);
    opl2.setInstrument(2, instruments[ 0 ]);
    opl2.setBlock(2, 4);
    timer_reset(100);
}

void music_setup() {
    tempo = 200;

    // Initialize 3 channels of the tune.
    for (int i = 0; i < 3; i++) {
        struct Tune channel;
        channel.data = tuneData[i];
        channel.channel = i;
        channel.octave = 4;
        channel.noteDuration = 50;
        channel.noteLength = 50;
        channel.releaseTime = 0;
        channel.nextNoteTime = timer_get();
        channel.index = 0;
        channel.instrument = 0;
        music[i] = channel;
    }

    // Setup channels 0, 1 and 2.
    opl2.setInstrument(0, instruments[ 0 ]);
    opl2.setBlock(0, 5);
    opl2.setInstrument(1, instruments[ 0 ]);
    opl2.setBlock(1, 4);
    opl2.setInstrument(2, instruments[ 0 ]);
    opl2.setBlock(2, 4);

    timer_setup(100);
}

void music_shutdown() {
    timer_shutdown();
    for (int i = 0; i < 3; i++) {
        if (opl2.getKeyOn(music[i].channel)) {
            opl2.setKeyOn(music[i].channel, false);
        }
    }
}

void music_loop() {
    bool busy = false;
    for (int i = 0; i < 3; i++) {
        if (timer_get() >= music[i].releaseTime && opl2.getKeyOn(music[i].channel)) {
            opl2.setKeyOn(music[i].channel, false);
        }
        if (timer_get() >= music[i].nextNoteTime && music[i].data[music[i].index] != 0) {
            parseTune(&music[i]);
        }
        if (music[i].data[music[i].index] != 0 || timer_get() < music[i].nextNoteTime) {
            busy = true;
        }
    }

    if (!busy) {
        music_set("", "", "");
    }
}


void parseTune(struct Tune *tune) {
    while (tune->data[tune->index] != 0) {

        if (tune->data[tune->index] == 'i' ) {
            tune->index++;
            tune->instrument = parseNumber(tune);
            break;
        } else
            // Decrease octave if greater than 1.
        if (tune->data[tune->index] == '<' && tune->octave > 1) {
            tune->octave--;

            // Increase octave if less than 7.
        } else if (tune->data[tune->index] == '>' && tune->octave < 7) {
            tune->octave++;

            // Set octave.
        } else if (tune->data[tune->index] == 'o' && tune->data[tune->index + 1] >= '1' &&
                   tune->data[tune->index + 1] <= '7') {
            tune->octave = tune->data[++tune->index] - 48;

            // Set default note duration.
        } else if (tune->data[tune->index] == 'l') {
            tune->index++;
            int duration = parseNumber(tune);
            if (duration != 0) tune->noteDuration = duration;

            // Set note length in percent.
        } else if (tune->data[tune->index] == 'm') {
            tune->index++;
            tune->noteLength = parseNumber(tune);

            // Set song tempo.
        } else if (tune->data[tune->index] == 't') {
            tune->index++;
            tempo = parseNumber(tune);

            // Pause.
        } else if (tune->data[tune->index] == 'p' || tune->data[tune->index] == 'r') {
            tune->index++;
            tune->nextNoteTime = timer_get() + parseDuration(tune);
            break;

            // Next character is a note A..G so play it.
        } else if (tune->data[tune->index] >= 'a' && tune->data[tune->index] <= 'g') {
            parseNote(tune);
            break;
        }

        tune->index++;
    }
}


void parseNote(struct Tune *tune) {
    // Get index of note in base frequenct table.
    char note = (tune->data[tune->index++] - 97) * 3;
    if (tune->data[tune->index] == '-') {
        note++;
        tune->index++;
    } else if (tune->data[tune->index] == '+') {
        note += 2;
        tune->index++;
    }

    // Get duration, set delay and play note.
    int duration = parseDuration(tune);
    tune->nextNoteTime = timer_get() + duration;
    tune->releaseTime = timer_get() + ((long) duration * tune->noteLength / 100);

    opl2.setKeyOn(tune->channel, false);
    opl2.setFrequency(tune->channel, opl2.getNoteFrequency(tune->channel, tune->octave, noteDefs[note]));
    opl2.setInstrument(tune->channel, instruments[ tune->instrument ]);
    opl2.setKeyOn(tune->channel, true);
}


int parseDuration(struct Tune *tune) {
    unsigned char duration = parseNumber(tune);
    if (duration == 0) {
        duration = tune->noteDuration;
    }

    // See whether we need to double the duration
    unsigned char base;
    if (tune->data[tune->index + 1] == '.') {
        ++tune->index;
        base = 6;
    } else {
        base = 4;
    }

    // Calculate note duration in timer ticks (0.01s)
    int ticks = 6000U * base / duration / tempo;
    return ticks;
}


int parseNumber(struct Tune *tune) {
    int number = 0;
    if (tune->data[tune->index] != 0 && tune->data[tune->index] >= '0' && tune->data[tune->index] <= '9') {
        while (tune->data[tune->index] != 0 && tune->data[tune->index] >= '0' && tune->data[tune->index] <= '9') {
            number = number * 10 + (tune->data[tune->index++] - 48);
        }
        tune->index--;
    }
    return number;
}

