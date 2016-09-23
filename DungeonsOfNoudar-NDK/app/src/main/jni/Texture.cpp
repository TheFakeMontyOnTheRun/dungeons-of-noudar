//
// Created by monty on 28/05/16.
//

#include <memory>
#include "NativeBitmap.h"
#include "Texture.h"

namespace odb {
	Texture::Texture( unsigned int textureId, std::shared_ptr<NativeBitmap> bitmap ) : mTextureId( textureId ), mNativeBitmap( bitmap ) {
	}
}