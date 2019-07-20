#include <cerrno>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>
#include <cstdio>

#include "android_asset_operations.h"

int android_read(void *cookie, char *buf, int size) {
    return AAsset_read((AAsset *) cookie, buf, size);
}

int android_write(void *cookie, const char *buf, int size) {
    return EACCES; // can't provide write access to the apk
}

fpos_t android_seek(void *cookie, fpos_t offset, int whence) {
    return AAsset_seek((AAsset *) cookie, offset, whence);
}

int android_close(void *cookie) {
    AAsset_close((AAsset *) cookie);
    return 0;
}


FILE *android_fopen(const char *fname, const char *mode, AAssetManager *assetManager) {
    if (mode[0] == 'w') {
        return nullptr;
    }

    AAsset *asset = AAssetManager_open(assetManager, fname, 0);

    if (!asset) {
        return nullptr;
    }

    return funopen(asset, android_read, android_write, android_seek, android_close);
}

