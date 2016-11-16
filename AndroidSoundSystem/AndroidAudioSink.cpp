//
// Created by monty on 15/10/16.
//

#include "AudioSink.h"
#include "AndroidAudioSink.h"
#include "JNIAudio.h"


AndroidAudioSink::AndroidAudioSink() {
	createAudioEngine();
	createBufferQueueAudioPlayer(44100, 512 );
}

int AndroidAudioSink::bufferData(unsigned char *data, int size, unsigned sampleRate, unsigned channels, unsigned bits) {
	return addAudioBuffer( data, size, sampleRate, channels, bits );
}

void AndroidAudioSink::play(int id, float volumeLeft, float volumeRight) {
	selectAudioClip( id, 1);
}

AndroidAudioSink::~AndroidAudioSink() {
	selectAudioClip( 0, 0 );
	setPlayingAssetAudioPlayer( false );
	shutdownAudio();
}











