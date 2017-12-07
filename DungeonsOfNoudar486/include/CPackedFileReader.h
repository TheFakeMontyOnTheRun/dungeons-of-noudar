//
// Created by monty on 06-12-2017.
//

#ifndef DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H
#define DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H

namespace odb {
    class CPackedFileReader : public Knights::IFileLoaderDelegate {
        FILE *mDataPack = nullptr;
        std::string mPackPath;
        std::unordered_map<std::string, size_t > mOffsets;
    public:
        CPackedFileReader() = delete;
        explicit CPackedFileReader( std::string dataFilePath );
        vector<char> loadBinaryFileFromPath( const std::string& path ) override;
        std::string loadFileFromPath( const std::string& path ) override ;
        std::string getFilePathPrefix() override;


    };
}

#endif //DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H
