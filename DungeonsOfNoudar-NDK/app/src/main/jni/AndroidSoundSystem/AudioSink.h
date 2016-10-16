//
// Created by monty on 15/10/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_AUDIOSINK_H
#define DUNGEONSOFNOUDAR_NDK_AUDIOSINK_H
class AudioSink {
public:
	virtual int bufferData(  unsigned char* data, int size, unsigned sampleRate, unsigned channels, unsigned bits ) = 0 ;
	virtual void play( int id, float volumeLeft, float volumeRight ) = 0;
};
#endif //DUNGEONSOFNOUDAR_NDK_AUDIOSINK_H
