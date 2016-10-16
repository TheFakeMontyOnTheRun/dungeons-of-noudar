//
// Created by monty on 15/10/16.
//
#include <memory>
#include "AudioSink.h"
#include "SoundClip.h"

odb::SoundClip::SoundClip(std::shared_ptr<AudioSink> audioSink, unsigned char *data, int size, unsigned offset, unsigned bits, unsigned channels,
                          unsigned frequency) : mAudioSink( audioSink ){

	mBufferHandle = mAudioSink->bufferData( &data[offset], size - offset, frequency, channels, bits );
}

