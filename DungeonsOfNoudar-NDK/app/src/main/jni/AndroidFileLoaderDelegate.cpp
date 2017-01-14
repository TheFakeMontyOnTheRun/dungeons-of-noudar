//
// Created by monty on 09/12/16.
//

#include <vector>

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>

#include <string>

#include "Common.h"

#include "IFileLoaderDelegate.h"
#include "AndroidFileLoaderDelegate.h"
#include "android_asset_operations.h"

namespace odb {
    AndroidFileLoaderDelegate::AndroidFileLoaderDelegate(AAssetManager *assetManager) : mAssetManager( assetManager ) {
    }

    std::vector<char> AndroidFileLoaderDelegate::loadBinaryFileFromPath( const std::string& path ) {
        FILE *fd;

        fd = android_fopen(path.c_str(), "rb", mAssetManager);
        std::vector<char> toReturn = readToBuffer(fd);
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
}
