//
// Created by monty on 06/10/16.
//
#include <memory>
#include <vector>
#include "IFileLoaderDelegate.h"
#include "NativeBitmap.h"
#include "LoadPNG.h"

#include "stb_image.h"

std::shared_ptr<odb::NativeBitmap> loadPNG(const std::string filename, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader ) {

    auto buffer = fileLoader->loadBinaryFileFromPath( filename );
    int xSize;
    int ySize;
    int components;
    auto image = stbi_load_from_memory((const stbi_uc *) buffer.data(), buffer.size(), &xSize, &ySize, &components, 0 );

    return std::make_shared<odb::NativeBitmap>( xSize, ySize, (int*)image );
}