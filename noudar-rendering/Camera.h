//
// Created by monty on 09/02/17.
//

#ifndef DUNGEONSOFNOUDAR_NDK_CAMERA_H
#define DUNGEONSOFNOUDAR_NDK_CAMERA_H

namespace odb {
	class Camera {

		glm::mat4 mTransform = glm::mat4(1.0f);

		glm::vec2 mCameraTarget;
		glm::vec2 cameraPosition;

		float *mEyeView = nullptr;

		float mAngleXZ = 0;
		float mAngleYZ = 0;

		int mCameraRotation = 0;
		int mRotationTarget = 0;

	public:
		void rotateBy(glm::vec3 axis, int degrees);

		void moveBy(glm::vec3 delta);

		void rotateTo(glm::vec3 axis, int degrees);

		void moveTo(glm::vec3 position);

		glm::mat4 getTransform();

		glm::vec3 getPosition();

		glm::mat4 getViewMatrix(glm::vec3 position);

		void update( long ms );

		void incrementRotateTarget( int delta );

		bool isAnimating();

		void setEyeView( float *eyeView );

		void reset();

		void setRotationXZ( float xz );

		void setRotationYZ( float yz );

		int getCameraRotationXZ() const;
	};
}

#endif //DUNGEONSOFNOUDAR_NDK_CAMERA_H
