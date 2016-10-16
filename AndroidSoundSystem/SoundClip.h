//
// Created by monty on 15/10/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_SOUNDCLIP_H
#define DUNGEONSOFNOUDAR_NDK_SOUNDCLIP_H

namespace odb {
	class SoundClip {
		std::shared_ptr<AudioSink> mAudioSink;
	public:
		unsigned int mBufferHandle;

		SoundClip(std::shared_ptr<AudioSink> audioSink, unsigned char *bufferData, int size, unsigned offset, unsigned bits, unsigned channels, unsigned frequency);
	};
}

#endif //DUNGEONSOFNOUDAR_NDK_SOUNDCLIP_H
