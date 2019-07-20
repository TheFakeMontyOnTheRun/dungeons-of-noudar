//
// Created by monty on 09/02/17.
//
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <memory>
#include <iostream>

#include <map>

#include "glm/gtc/type_ptr.hpp"

#include "Camera.h"

const static bool kUseQuarterAngles = false;

const static bool kCompleteMovementInstantly = false;


static const auto transformIdentity = glm::mat4(1.0f);

glm::mat4 odb::Camera::getTransform() {
    return mTransform;
}

void odb::Camera::rotateBy(glm::vec3 axis, int degrees) {
    mTransform = glm::rotate(mTransform, (float) degrees, axis);
}

void odb::Camera::rotateTo(glm::vec3 axis, int degrees) {
    mTransform = glm::rotate(transformIdentity, (float) degrees, axis);
}

void odb::Camera::moveBy(glm::vec3 delta) {
    mTransform = glm::translate(mTransform, delta);
}

void odb::Camera::moveTo(glm::vec3 position) {
    mTransform = glm::translate(transformIdentity, position);
}

glm::vec3 odb::Camera::getPosition() {
    return {mTransform[3][0],
            mTransform[3][1],
            mTransform[3][2]
    };
}

void odb::Camera::update(long ms) {
    cameraPosition.x += (ms * (mCameraTarget.x - cameraPosition.x)) / 1000;
    cameraPosition.y += (ms * (mCameraTarget.y - cameraPosition.y)) / 1000;

    if (mRotationTarget > mCameraRotation) {


        if (kUseQuarterAngles) {
            mCameraRotation = std::min(mCameraRotation + 45, mRotationTarget);
        } else {
            mCameraRotation = std::min(mCameraRotation + 5, mRotationTarget);
        }
    } else if (mRotationTarget < mCameraRotation) {
        if (kUseQuarterAngles) {
            mCameraRotation = std::max(mCameraRotation - 45, mRotationTarget);
        } else {
            mCameraRotation = std::max(mCameraRotation - 5, mRotationTarget);
        }
    }
}

void odb::Camera::incrementRotateTarget(int delta) {
    this->mRotationTarget += delta;

    if (kCompleteMovementInstantly) {
        mCameraRotation += delta;
    } else {
        if (kUseQuarterAngles) {
            mCameraRotation += delta / 2;
        }
    }
}

bool odb::Camera::isAnimating() {
    return mRotationTarget != mCameraRotation;
}

void odb::Camera::setEyeView(float *eyeView) {
    mEyeView = eyeView;
}

void odb::Camera::reset() {
    mAngleXZ = 0;
    mAngleYZ = 0;
    mCameraRotation = 0;
    mRotationTarget = 0;
}

void odb::Camera::setRotationXZ(float xz) {
    mAngleXZ = xz;
}

void odb::Camera::setRotationYZ(float yz) {
    mAngleYZ = yz;
}

int odb::Camera::getCameraRotationXZ() const {
    return (mCameraRotation) + mAngleXZ;
}

glm::mat4 odb::Camera::getViewMatrix(glm::vec3 position) {

    if (!kUseQuarterAngles) {
        glm::vec3 pos = position;
        glm::vec4 pos_front4 = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
        glm::vec3 pos_front;
        glm::mat4 eyeMatrixOriginal =
                mEyeView != nullptr ? glm::make_mat4(mEyeView) : glm::mat4(1.0f);
        glm::mat4 eyeMatrix = glm::mat4(1.0f);

        eyeMatrix[3][0] = eyeMatrixOriginal[3][0];
        eyeMatrix[3][1] = eyeMatrixOriginal[3][1];
        eyeMatrix[3][2] = eyeMatrixOriginal[3][2];

        float angleInRadiansYZ = mAngleYZ * (3.14159f / 180.0f);
        float angleInRadiansXZ = (mAngleXZ - mCameraRotation) * (3.14159f / 180.0f);

        glm::vec3 mCameraDirection{0, 0, 0};

        auto direction4 = glm::rotate(
                glm::rotate(glm::mat4(1.0f), angleInRadiansXZ, glm::vec3(0.0f, 1.0f, 0.0f)),
                angleInRadiansYZ, glm::vec3(1.0f, 0.0f, 0.0f)) * pos_front4;

        mCameraDirection = glm::vec3(direction4.x, direction4.y, direction4.z);

        pos_front = mCameraDirection;

        const glm::vec3 forward{
                pos_front.x + pos.x,
                pos_front.y + pos.y,
                pos_front.z + pos.z
        };


        return glm::lookAt(
                pos,
                forward,
                glm::vec3(0.0f, 1.0, 0.0f)) * eyeMatrix;
    } else {

        int direction = (mCameraRotation / 45);

        direction = direction % 8;

        while (direction < 0) {
            direction += 8;
        }

        static const glm::vec3 forward[8]{
                glm::vec3(0.0f, 0.0f, -1.0f),
                glm::vec3(1.0f, 0.0f, -1.0f),
                glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(1.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(-1.0f, 0.0f, 1.0f),
                glm::vec3(-1.0f, 0.0f, 0.0f),
                glm::vec3(-1.0f, 0.0f, -1.0f),
        };

        return glm::lookAt(
                position,
                position + forward[direction],
                glm::vec3(0.0f, 1.0, 0.0f));

    }
}








