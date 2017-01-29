//
// Created by monty on 22/10/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_SPLATANIMATION_H
#define DUNGEONSOFNOUDAR_NDK_SPLATANIMATION_H


namespace odb {
	class SplatAnimation {

	public:
		SplatAnimation( Knights::Vec2i position );

		void update(long ms);

		void startSplatAnimation();

		int getSplatFrame();

		bool isFinished();

		Knights::Vec2i getPosition();
	private:
#ifndef OSMESA
		const int TOTAL_ANIMATION_TIME = 200;
#else
		const int TOTAL_ANIMATION_TIME = 1000;
#endif
		const int NUMBER_OF_FRAMES = 3;
		Knights::Vec2i mPosition{ 0,0 };

		long showSplatTime = 0;
	};
}

#endif //DUNGEONSOFNOUDAR_NDK_SPLATANIMATION_H
