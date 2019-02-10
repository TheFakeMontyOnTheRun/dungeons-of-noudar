//
// Created by monty on 09/12/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_ANDROIDFILELOADERDELEGATE_H
#define DUNGEONSOFNOUDAR_NDK_ANDROIDFILELOADERDELEGATE_H


namespace odb {
    class AndroidFileLoaderDelegate : public Knights::IFileLoaderDelegate {
        AAssetManager *mAssetManager;
    public:
        AndroidFileLoaderDelegate( AAssetManager *assetManager );
        std::string getFilePathPrefix() override;
        uint8_t* loadBinaryFileFromPath( const std::string& path ) override;
        std::string loadFileFromPath( const std::string& path ) override;
        size_t sizeOfFile(const std::string& path) override;
    };
}

#endif //DUNGEONSOFNOUDAR_NDK_ANDROIDFILELOADERDELEGATE_H
