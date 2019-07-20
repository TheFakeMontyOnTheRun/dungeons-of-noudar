//
// Created by monty on 28/02/16.
//

#ifndef LESSON10_NATIVEBITMAP_H
#define LESSON10_NATIVEBITMAP_H
namespace odb {
	class NativeBitmap {
		std::string mId;
		int mWidth;
		int mHeight;
		int *mRawData;

	public:
		NativeBitmap( std::string aId, int aWidth, int aHeight, int *aRawData);

		std::shared_ptr<NativeBitmap> makeBitmapWithHalfDimensions() const;

		int getWidth() const;

		int getHeight() const;

		int *getPixelData() const;

		std::string getId() const;

		void releaseTextureData();

		~NativeBitmap();
	};
}

#endif //LESSON10_NATIVEBITMAP_H
