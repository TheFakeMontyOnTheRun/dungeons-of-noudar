//
// Created by monty on 15/10/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_SOUNDEMITTER_H
#define DUNGEONSOFNOUDAR_NDK_SOUNDEMITTER_H

namespace odb {
	class SoundEmitter {
		std::shared_ptr <SoundClip> mSample = nullptr;
		unsigned int mEmitterHandle;
		std::shared_ptr<AudioSink> mAudioSink;
	public:
		glm::vec3 mPosition{0, 0, 0};
		float mVolume = 1.0f;

		SoundEmitter( std::shared_ptr<AudioSink> audioSink, std::shared_ptr<SoundClip> aSample );
		void play( std::shared_ptr<SoundListener> listener );
	};
}

#endif //DUNGEONSOFNOUDAR_NDK_SOUNDEMITTER_H
