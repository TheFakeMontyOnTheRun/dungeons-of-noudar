//
// Created by monty on 15/10/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_AUDIOSINKWRAPPER_H
#define DUNGEONSOFNOUDAR_NDK_AUDIOSINKWRAPPER_H

class AndroidAudioSink: public AudioSink {
	jobject mSink;
	JNIEnv *mEnv;
	jclass mClass;
	jmethodID mBufferDataMethod;
	jmethodID mPlayMethod;

	std::vector< int > mQueuedItems;
public:
	void updateEnv( JNIEnv *env, jclass type, jobject sink );
	void flush(JNIEnv *env, jclass type, jobject sink);
	AndroidAudioSink(JNIEnv *env, jclass type, jobject sink);
	int bufferData(  unsigned char* data, int size, unsigned sampleRate, unsigned channels, unsigned bits ) override;
	void play( int id, float volumeLeft, float volumeRight ) override;
};


#endif //DUNGEONSOFNOUDAR_NDK_AUDIOSINKWRAPPER_H
