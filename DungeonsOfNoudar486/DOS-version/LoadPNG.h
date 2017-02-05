//
// Created by monty on 06/10/16.
//

#ifndef NOUDAR_CORE_LOADPNG_H
#define NOUDAR_CORE_LOADPNG_H

using VGApalette = std::array<int, 256>;
std::shared_ptr<odb::NativeBitmap> loadPNG(const std::string filename);
std::shared_ptr<odb::NativeBitmap> loadPNG(const std::string filename, int w, int h);

VGApalette extractPalette( std::shared_ptr<odb::NativeBitmap> );
VGApalette mergePalettes( std::vector< VGApalette > palettes );
#endif //NOUDAR_CORE_LOADPNG_H
