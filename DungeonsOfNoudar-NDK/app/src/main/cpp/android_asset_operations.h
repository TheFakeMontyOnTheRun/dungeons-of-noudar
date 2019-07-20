#ifndef ANDROID_ASSETS_H
#define ANDROID_ASSETS_H

static int android_read(void *cookie, char *buf, int size);

static int android_write(void *cookie, const char *buf, int size);

static fpos_t android_seek(void *cookie, fpos_t offset, int whence);

static int android_close(void *cookie);


FILE *android_fopen(const char *fname, const char *mode, AAssetManager *assetManager);

#endif
