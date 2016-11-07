/*
 Copyright (C) 2015 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 Functions for loading an image files for textures.
 */


#ifndef __IMAGE_UTIL_H__
#define __IMAGE_UTIL_H__

std::shared_ptr<odb::NativeBitmap> loadPNG(const char* filepathname);

#endif //__IMAGE_UTIL_H__

