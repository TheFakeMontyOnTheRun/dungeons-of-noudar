//
// Created by monty on 10/10/16.
//
#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "SoundClip.h"
#include "SoundUtils.h"

namespace odb {
    std::shared_ptr<SoundClip> makeSoundClipFrom(FILE* source) {
        fseek(source, 0, SEEK_END);
        int size = ftell(source);
        printf( "size: %d\n", size );
        fseek(source, 0, SEEK_SET);

        using bufferElement = unsigned char;

        auto buffer = new bufferElement[ size ];

        fread(buffer, size, 1, source);

        unsigned offset = 12; // ignore the RIFF header
        offset += 8; // ignore the fmt header
        offset += 2; // ignore the format type

        unsigned channels = buffer[offset + 1] << 8;
        channels |= buffer[offset];
        offset += 2;

        unsigned frequency = buffer[offset + 3] << 24;
        frequency |= buffer[offset + 2] << 16;
        frequency |= buffer[offset + 1] << 8;
        frequency |= buffer[offset];
        offset += 4;
        offset += 6; // ignore block size and bps

        unsigned bits = buffer[offset + 1] << 8;
        bits |= buffer[offset];
        offset += 2;
        offset += 8; // ignore the data chunk
        printf("Start offset: %d\n", offset);
        printf("Channels: %u\n", channels);
        printf("Frequency: %u\n", frequency);
        printf("Bits: %u\n", bits);
        return std::make_shared<SoundClip>( buffer, size, offset, bits, channels, frequency );
    }
}
