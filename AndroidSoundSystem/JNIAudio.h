//
// Created by Daniel Monteiro on 15/11/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_JNIAUDIO_H
#define DUNGEONSOFNOUDAR_NDK_JNIAUDIO_H
void createAudioEngine();
void createBufferQueueAudioPlayer( int sampleRate, int bufSize);
void shutdownAudio();
int addAudioBuffer(unsigned char *data, int size, unsigned sampleRate, unsigned channels, unsigned bits);
bool selectAudioClip( int which, int count );
void setPlayingAssetAudioPlayer(bool isPlaying);
#endif //DUNGEONSOFNOUDAR_NDK_JNIAUDIO_H
