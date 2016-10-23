//
// Created by monty on 15/10/16.
//

#include <vector>

#include <jni.h>
#include "AudioSink.h"
#include "AndroidAudioSink.h"

AndroidAudioSink::AndroidAudioSink(JNIEnv *env, jclass type, jobject sink) : mEnv( env ), mSink( sink ) {
	mClass = env->FindClass("br/odb/SoundSink");
	mBufferDataMethod = env->GetMethodID(mClass, "bufferData", "([BIII)I");
	mPlayMethod = env->GetMethodID(mClass, "play", "(IFF)V");
}

int AndroidAudioSink::bufferData(unsigned char *data, int size, unsigned sampleRate, unsigned channels, unsigned bits) {
	jbyteArray javaByteArray;
	javaByteArray = mEnv->NewByteArray(size);

	if( !javaByteArray || mEnv->GetArrayLength(javaByteArray) != size) {

		if ( javaByteArray ) {
			mEnv->DeleteLocalRef( javaByteArray);
		}

		javaByteArray = mEnv->NewByteArray( size);
	}

	void *temp = mEnv->GetPrimitiveArrayCritical((jarray)javaByteArray, 0);
	memcpy(temp, data, size);
	mEnv->ReleasePrimitiveArrayCritical(javaByteArray, temp, 0);

	return mEnv->CallIntMethod( mSink, mBufferDataMethod, javaByteArray, sampleRate, channels, bits );
}

void AndroidAudioSink::play(int id, float volumeLeft, float volumeRight) {
	mQueuedItems.push_back( id );
}

void AndroidAudioSink::updateEnv(JNIEnv *env, jclass type, jobject sink ) {
	mEnv = env;
	mSink = sink;
}

void AndroidAudioSink::flush(JNIEnv *env, jclass type, jobject sink) {

	mEnv = env;
	mSink = sink;

	mClass = env->FindClass("br/odb/SoundSink");
	mBufferDataMethod = env->GetMethodID(mClass, "bufferData", "([BIII)I");
	mPlayMethod = env->GetMethodID(mClass, "play", "(IFF)V");

	for ( auto id : mQueuedItems ) {
		env->CallVoidMethod( mSink, mPlayMethod, id, 1.0f, 1.0f );
	}

	mQueuedItems.clear();
}









