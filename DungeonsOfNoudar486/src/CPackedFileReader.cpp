//
// Created by monty on 06-12-2017.
//
#include <string>
#include <unordered_map>
#include <vector>

using std::vector;

#include "Common.h"
#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"


#ifdef ANDROID
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>

extern AAssetManager *defaultAssetManager;

int android_read(void *cookie, char *buf, int size) {
    return AAsset_read((AAsset *) cookie, buf, size);
}

int android_write(void *cookie, const char *buf, int size) {
    return EACCES;
}

fpos_t android_seek(void *cookie, fpos_t offset, int whence) {
    return AAsset_seek((AAsset *) cookie, offset, whence);
}

int android_close(void *cookie) {
    AAsset_close((AAsset *) cookie);
    return 0;
}


FILE *android_fopen(const char* filename) {

    AAsset *asset = AAssetManager_open(defaultAssetManager, "data.pfs", 0);
    if (!asset) {
        return NULL;
    }

    return funopen(asset, android_read, android_write, android_seek, android_close);

}
#endif

odb::CPackedFileReader::CPackedFileReader(std::string dataFilePath) : mPackPath(dataFilePath) {
#ifndef ANDROID
    FILE *mDataPack = fopen(mPackPath, "rb");
#else
    FILE *mDataPack = android_fopen(&mPackPath[0]);
#endif

    uint16_t entries = 0;
    fread(&entries, 2, 1, mDataPack);

    char buffer[85];

    for ( int c = 0; c < entries; ++c ) {

        uint32_t offset = 0;
        fread(&offset, 4, 1, mDataPack );

        uint8_t stringSize = 0;
        fread(&stringSize, 1, 1, mDataPack );

        fread(&buffer, stringSize + 1, 1, mDataPack );
        std::string name = buffer;

#ifdef SDLSW
//        std::cout << "name " << name << " offset " << offset << std::endl;
#endif
        mOffsets[name] = offset;
    }

    fclose(mDataPack);
}

size_t odb::CPackedFileReader::sizeOfFile(const std::string& path) {
#ifndef ANDROID
    FILE *mDataPack = fopen(mPackPath, "rb");
#else
    FILE *mDataPack = android_fopen(&mPackPath[0]);
#endif

    uint32_t offset = mOffsets[ path ];
    if ( offset == 0 ) {
        printf("failed to load %s", path.c_str());
        exit(-1);
    }

    auto result = fseek( mDataPack, offset, SEEK_SET );

    uint32_t size = 0;
    fread(&size, 4, 1, mDataPack );
    fclose(mDataPack);

    return size;
}

uint8_t* odb::CPackedFileReader::loadBinaryFileFromPath(const std::string &path) {
    uint8_t* toReturn;
#ifndef ANDROID
    FILE *mDataPack = fopen(mPackPath, "rb");
#else
    FILE *mDataPack = android_fopen(&mPackPath[0]);
#endif

    uint32_t offset = mOffsets[ path ];
    if ( offset == 0 ) {
        printf("failed to load %s", path.c_str());
        exit(-1);
    }

    auto result = fseek( mDataPack, offset, SEEK_SET );

    uint32_t size = 0;
    char data;
    fread(&size, 4, 1, mDataPack );
    toReturn = new uint8_t[size];

    auto ptr = toReturn;
    for ( int c = 0; c < size; ++c ) {
        fread(&data, 1, 1, mDataPack );
        (*ptr++)=(data);
    }
    fclose(mDataPack);

    return toReturn;
}

std::string odb::CPackedFileReader::loadFileFromPath(const std::string &path) {
    std::string toReturn;
#ifndef ANDROID
    FILE *mDataPack = fopen(mPackPath, "rb");
#else
    FILE *mDataPack = android_fopen(&mPackPath[0]);
#endif

    uint32_t offset = mOffsets[ path ];
    if ( offset == 0 ) {
        printf("failed to load %s", path.c_str());
        exit(-1);
    }
    auto result = fseek( mDataPack, offset, SEEK_SET );

    uint32_t size = 0;
    char data;
    fread(&size, 4, 1, mDataPack );

    for ( int c = 0; c < size; ++c ) {
        fread(&data, 1, 1, mDataPack );
        toReturn.push_back(data);
    }
    fclose(mDataPack);

    return toReturn;
}

std::string odb::CPackedFileReader::getFilePathPrefix() {
    return "";
}

odb::CPackedFileReader::~CPackedFileReader() {

}