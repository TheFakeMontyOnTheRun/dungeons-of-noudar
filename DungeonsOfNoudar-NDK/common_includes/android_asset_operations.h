#include <cerrno>

static int android_read(void *cookie, char *buf, int size) {
    return AAsset_read((AAsset *) cookie, buf, size);
}

static int android_write(void *cookie, const char *buf, int size) {
    return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek(void *cookie, fpos_t offset, int whence) {
    return AAsset_seek((AAsset *) cookie, offset, whence);
}

static int android_close(void *cookie) {
    AAsset_close((AAsset *) cookie);
    return 0;
}


FILE *android_fopen(const char *fname, const char *mode, AAssetManager *assetManager) {
    if (mode[0] == 'w') return NULL;

    AAsset *asset = AAssetManager_open(assetManager, fname, 0);
    if (!asset) return NULL;

    return funopen(asset, android_read, android_write, android_seek, android_close);
}

std::string readToString(FILE *fileDescriptor) {
    const unsigned N = 1024;
    std::string total;
    while (true) {
        char buffer[N];
        size_t read = fread((void *) &buffer[0], 1, N, fileDescriptor);
        if (read) {
            for (int c = 0; c < read; ++c) {
                total.push_back(buffer[c]);
            }
        }
        if (read < N) {
            break;
        }
    }

    return total;
}

