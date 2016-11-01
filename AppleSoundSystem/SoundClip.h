//
// Created by monty on 10/10/16.
//
#ifndef TIC_TAC_TOE_GLES_SOUNDCLIP_H
#define TIC_TAC_TOE_GLES_SOUNDCLIP_H

namespace odb {
    class SoundClip {
    public:
        unsigned int mBufferHandle;

        SoundClip(unsigned char *bufferData, int size, int offset, int bits, int channels, int frequency);
    };
}

#endif //TIC_TAC_TOE_GLES_SOUNDCLIP_H
