//
// Created by monty on 28/05/16.
//

#ifndef KNIGHTSOFALENTEJOANDROID_AS_TEXTURE_H
#define KNIGHTSOFALENTEJOANDROID_AS_TEXTURE_H

namespace odb {
	class Texture {
	public:
		unsigned int mTextureId;
		int mWidth;
		int mHeight;
		std::shared_ptr<NativeBitmap> mNativeBitmap;
		Texture( unsigned int textureId, std::shared_ptr<NativeBitmap> bitmap );
	};
}

#endif //KNIGHTSOFALENTEJOANDROID_AS_TEXTURE_H
