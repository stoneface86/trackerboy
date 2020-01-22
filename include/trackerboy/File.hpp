
#pragma once

#include "trackerboy/fileformat.hpp"


namespace trackerboy {

//
// Container class for information stored in a trackerboy file.
//
class File {

public:
    File(std::string path);
    ~File();

    //
    // Reads from the file and loads header data if the file is valid. The
    // chunk data can then be read by the caller afterwards.
    //
    std::ifstream load(std::string path, FormatError &error);

    //
    // Writes the header and terminator to the file. The chunk data is to be
    // written by the caller afterwards.
    //
    std::ofstream save(std::string path, FormatError &error);

    // upgrades a loaded file from an older revision to the current
    //void upgrade();

    void setArtist(std::string artist);

    void setCopyright(std::string copyright);

    void setTitle(std::string title);

    void setChunkSize(uint32_t size);

    void setChunkType(ChunkType type);

    uint8_t revision();

    std::string title();
    
    std::string artist();

    std::string copyright();

    ChunkType chunkType();

    uint32_t chunkSize();

private:

    // header settings
    uint8_t mRevision;
    std::string mTitle;
    std::string mArtist;
    std::string mCopyright;
    ChunkType mChunkType;
    uint32_t mChunkSize;


};



}