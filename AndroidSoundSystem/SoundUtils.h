//
// Created by monty on 15/10/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_SOUNDUTILS_H
#define DUNGEONSOFNOUDAR_NDK_SOUNDUTILS_H
namespace odb {
	std::shared_ptr<SoundClip> makeSoundClipFrom(std::shared_ptr<AudioSink> audioSink, FILE* fileHandle);
}
#endif //DUNGEONSOFNOUDAR_NDK_SOUNDUTILS_H
