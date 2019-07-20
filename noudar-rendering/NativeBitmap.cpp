////
//// Created by monty on 28/02/16.
////
#include <memory>
#include <string>
#include "NativeBitmap.h"

namespace odb {
    int NativeBitmap::getWidth() const {
        return this->mWidth;
    }

    int NativeBitmap::getHeight() const {
        return this->mHeight;
    }

    int *NativeBitmap::getPixelData() const {
        return this->mRawData;
    }

    std::string NativeBitmap::getId() const {
        return mId;
    }

    NativeBitmap::NativeBitmap(std::string aId, int aWidth, int aHeight, int *aRawData) :
            mId(aId), mWidth(aWidth), mHeight(aHeight), mRawData(aRawData) {
    }

    void NativeBitmap::releaseTextureData() {
        this->mWidth = 0;
        this->mHeight = 0;
        this->mRawData = nullptr;
    }

    NativeBitmap::~NativeBitmap() {
        delete[] mRawData;
    }

    std::shared_ptr<NativeBitmap> NativeBitmap::makeBitmapWithHalfDimensions() const {
        int *rawData;
        int newWidth = mWidth / 2;
        int newHeight = mHeight / 2;
        rawData = new int[newHeight * newWidth];

        for (int y = 0; y < mHeight; ++y) {
            for (int x = 0; x < mWidth; ++x) {
                rawData[(newWidth * (y / 2)) + (x / 2)] = this->mRawData[(mWidth * y) + x];
            }
        }

        return std::make_shared<NativeBitmap>( mId, newWidth, newHeight, rawData);
    }
}