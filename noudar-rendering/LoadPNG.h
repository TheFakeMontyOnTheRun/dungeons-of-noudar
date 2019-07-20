//
// Created by monty on 06/10/16.
//

#ifndef NOUDAR_CORE_LOADPNG_H
#define NOUDAR_CORE_LOADPNG_H

std::shared_ptr<odb::NativeBitmap> loadPNG(const std::string filename, std::shared_ptr<Knights::IFileLoaderDelegate> fileLoader);

#endif //NOUDAR_CORE_LOADPNG_H
