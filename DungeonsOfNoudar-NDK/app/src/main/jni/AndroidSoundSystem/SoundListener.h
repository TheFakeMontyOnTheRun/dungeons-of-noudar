//
// Created by monty on 15/10/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_SOUNDLISTENER_H
#define DUNGEONSOFNOUDAR_NDK_SOUNDLISTENER_H


namespace odb {
	class SoundListener {
		std::shared_ptr<AudioSink> mAudioSink;
	public:
		glm::vec3 mPosition{0, 0, 0};
		SoundListener( std::shared_ptr<AudioSink>);
	};
}

#endif //DUNGEONSOFNOUDAR_NDK_SOUNDLISTENER_H
