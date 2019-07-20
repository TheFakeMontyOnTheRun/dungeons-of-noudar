//
// Created by Daniel Monteiro on 12/11/16.
//
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "VBORenderingJob.h"

glm::mat4 odb::VBORenderingJob::getTransform() const {
    return mVBOInstanceTransform;
}

int odb::VBORenderingJob::getVBOId() const {
    return mVBOId;
}

odb::VBORenderingJob::VBORenderingJob(int aVBOId, int aVBOIndicesId, int aAmount, glm::mat4 aTransform, float aShade, bool needsAlphaTest = false) :
        mVBOInstanceTransform( aTransform),
        mShade(aShade),
        mVBOId(aVBOId),
        mVBOIndicesId(aVBOIndicesId),
        mAmount(aAmount),
        mNeedsAlphaTest( needsAlphaTest ){
}

float odb::VBORenderingJob::getShade() const {
    return mShade;
}

int odb::VBORenderingJob::getAmount() const {
    return mAmount;
}

int odb::VBORenderingJob::getVBOIndicesId() const {
    return mVBOIndicesId;
}











