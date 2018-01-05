#ifndef DEMOTUNE_H_
#define DEMOTUNE_H_
extern void music_set(const char* melody1, const char* melody2, const char* melody3);
extern void music_setup();
extern void music_loop();
extern void music_shutdown();
extern void hackTune(const char* tune);
//instrument used for sfx - a quite misleading name...
extern int music_instrument;
#endif
