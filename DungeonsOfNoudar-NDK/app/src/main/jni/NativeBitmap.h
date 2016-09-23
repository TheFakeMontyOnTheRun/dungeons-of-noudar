//
// Created by monty on 28/02/16.
//

#ifndef LESSON10_NATIVEBITMAP_H
#define LESSON10_NATIVEBITMAP_H
namespace odb {
	class NativeBitmap {
		int mWidth;
		int mHeight;
		int *mRawData;

	public:
		NativeBitmap(int aWidth, int aHeight, int *aRawData);

		std::shared_ptr<NativeBitmap> makeBitmapWithHalfDimensions();

		int getWidth();

		int getHeight();

		int *getPixelData();

		void releaseTextureData();

		~NativeBitmap();
	};
}

#endif //LESSON10_NATIVEBITMAP_H
