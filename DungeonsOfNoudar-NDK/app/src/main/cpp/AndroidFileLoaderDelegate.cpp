//
// Created by monty on 09/12/16.
//

#include <vector>
#include <array>

using std::vector;
using std::array;


#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>

#include <string>
#include <cstdint>
#include "Common.h"

#include "IFileLoaderDelegate.h"
#include "AndroidFileLoaderDelegate.h"
#include "android_asset_operations.h"

using Knights::readToBuffer;
using Knights::readToString;

namespace odb {
    AndroidFileLoaderDelegate::AndroidFileLoaderDelegate(AAssetManager *assetManager) : mAssetManager( assetManager ) {
    }

    uint8_t* AndroidFileLoaderDelegate::loadBinaryFileFromPath( const std::string& path ) {
        FILE *fd;

        fd = android_fopen(path.c_str(), "rb", mAssetManager);
        auto toReturn = readToBuffer(fd);
        fclose(fd);

        return toReturn;
    }

    std::string AndroidFileLoaderDelegate::loadFileFromPath( const std::string& path ) {
        FILE *fd;

        fd = android_fopen(path.c_str(), "r", mAssetManager);
        std::string toReturn = readToString(fd);
        fclose(fd);

        return toReturn;
    }

    std::string AndroidFileLoaderDelegate::getFilePathPrefix() {
        return "";
    }


    size_t AndroidFileLoaderDelegate::sizeOfFile(const std::string& path) {
        FILE *fd;

        fd = android_fopen(path.c_str(), "r", mAssetManager);

        fseek(fd, 0, SEEK_END);
        auto endPos = ftell( fd );
        fclose(fd);

        return endPos;
    }
}
