//
// Created by monty on 06/10/16.
//
#include <memory>
#include <string>

#include <cstdlib>
#include <cstring>
#include <vector>
#include <array>

using std::vector;
using std::array;

#include "IFileLoaderDelegate.h"
#include "NativeBitmap.h"
#include "LoadPNG.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

std::shared_ptr<odb::NativeBitmap> loadPNG(const std::string filename, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {


    auto buffer = fileLoader->loadBinaryFileFromPath( filename );
    auto bufferSize = fileLoader->sizeOfFile(filename);
    int xSize;
    int ySize;
    int components;
#if defined(TARGET_IOS)
    stbi_convert_iphone_png_to_rgb(1);
#endif
    
    auto image = stbi_load_from_memory((const stbi_uc *) buffer, bufferSize, &xSize, &ySize, &components, 0 );
    auto rawData = new int[ xSize * ySize ];
    std::memcpy( rawData, image, xSize * ySize * 4 );
    stbi_image_free(image);

#if defined(OSMESA) || defined(VGA)
    for ( int c = 0; c < xSize * ySize; ++c  ) {
        int origin = rawData[ c ];

        int r = (origin & 0xFF000000) >> 24;
        int g = (origin & 0x00FF0000) >> 16;
        int b = (origin & 0x0000FF00) >> 8;
        int a = (origin & 0x000000FF);

        int r20 = (( r / 21 ) * 21 );
        int g20 = (( g / 21 ) * 21 );
        int b20 = (( b / 21) * 21 );

        int pixel = a;
        pixel += ( r20 << 24 );
        pixel += ( g20 << 16 );
        pixel += ( b20 <<  8 );

        rawData[ c ] = pixel;
    }
#endif

    delete[] buffer;
    buffer = nullptr;

    return std::make_shared<odb::NativeBitmap>( filename, xSize, ySize, rawData );
}
