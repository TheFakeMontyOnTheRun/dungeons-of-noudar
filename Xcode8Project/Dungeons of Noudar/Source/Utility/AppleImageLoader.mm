/*
 Copyright (C) 2015 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 Functions for loading an image files for textures.
 */
#include <memory>

#include "NativeBitmap.h"
#include "AppleImageLoader.h"

#if TARGET_IOS
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

std::shared_ptr<odb::NativeBitmap> loadPNG(const char* filepathname) {
	NSString *filepathString = [[NSString alloc] initWithUTF8String:filepathname];
	
	int *data;
	int width = 0;
	int height = 0;
	int rowByteSize = 0;
	
#if TARGET_IOS
	UIImage* imageClass = [[UIImage alloc] initWithContentsOfFile:filepathString];
#else   
    NSImage *nsimage = [[NSImage alloc] initWithContentsOfFile: filepathString];
	
	NSBitmapImageRep *imageClass = [[NSBitmapImageRep alloc] initWithData:[nsimage TIFFRepresentation]];
    nsimage = nil;
#endif
	
	CGImageRef cgImage = imageClass.CGImage;
	
	if (!cgImage) {
		return nullptr;
	}
	
	width = (GLuint)CGImageGetWidth(cgImage);
	height = (GLuint)CGImageGetHeight(cgImage);
	rowByteSize = width * 4;
	data = (int*)malloc(height * rowByteSize );
	
	CGContextRef context = CGBitmapContextCreate(data, width, height, 8, rowByteSize, CGImageGetColorSpace(cgImage), kCGBitmapAlphaInfoMask & kCGImageAlphaNoneSkipLast);
	CGContextSetBlendMode(context, kCGBlendModeCopy);

	
	auto image = std::make_shared<odb::NativeBitmap>( width, height, data );
	
	
	bool flipVertical = false;
	
	if(flipVertical) {
		CGContextTranslateCTM(context, 0.0, height);
		CGContextScaleCTM(context, 1.0, -1.0);
	}
	
	CGContextDrawImage(context, CGRectMake(0.0, 0.0, width, height), cgImage);
	CGContextRelease(context);
	
	return image;
}

