//
// Created by monty on 15/10/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_AUDIOSINKWRAPPER_H
#define DUNGEONSOFNOUDAR_NDK_AUDIOSINKWRAPPER_H

class AndroidAudioSink: public AudioSink {
public:
	AndroidAudioSink();
	int bufferData(  unsigned char* data, int size, unsigned sampleRate, unsigned channels, unsigned bits ) override;
	void play( int id, float volumeLeft, float volumeRight ) override;
	~AndroidAudioSink();
};


#endif //DUNGEONSOFNOUDAR_NDK_AUDIOSINKWRAPPER_H
