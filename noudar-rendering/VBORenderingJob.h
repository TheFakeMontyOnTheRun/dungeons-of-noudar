//
// Created by Daniel Monteiro on 12/11/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_VBORENDERINGJOB_H
#define DUNGEONSOFNOUDAR_NDK_VBORENDERINGJOB_H

namespace odb {
    class VBORenderingJob {
    public:
        explicit VBORenderingJob( int vboId, int vboIndicesId, int amount, glm::mat4 transform, float shade, bool needsAlphaTest);
        glm::mat4 getTransform() const;
        int getVBOId() const;
        int getVBOIndicesId() const;
        float getShade() const;
        int getAmount() const;
    private:
        glm::mat4 mVBOInstanceTransform = glm::mat4( 1.0f );
        float mShade = 1.0f;
        int mVBOId = -1;
        int mVBOIndicesId = -1;
        int mAmount = 0;
    public:
	    bool mNeedsAlphaTest = false;
    };
}

#endif //DUNGEONSOFNOUDAR_NDK_VBORENDERINGJOB_H
