
#pragma once

#include "trackerboy/fileformat.hpp"
#include "trackerboy/Table.hpp"


namespace trackerboy {

//
// Container class for information stored in a trackerboy file.
//
class File {

public:
    File();
    ~File();

    //
    // Reads just the header from the stream and updates header settings if
    // valid. FormatError::none is returned on success.
    //
    FormatError loadHeader(std::istream &stream);

    //
    // Writes just the header to the given output stream. FormatError::writeError
    // is returned if an io error occured with the given stream.
    //
    FormatError saveHeader(std::ostream &stream);

    template <class T>
    FormatError saveTable(std::ostream &stream, Table<T> &table);

    // upgrades a loaded file from an older revision to the current
    //void upgrade();

    void setArtist(std::string artist);

    void setCopyright(std::string copyright);

    void setTitle(std::string title);

    void setChunkType(ChunkType type);

    uint8_t revision();

    std::string title();
    
    std::string artist();

    std::string copyright();

    ChunkType chunkType();


private:
    // header settings
    uint8_t mRevision;
    std::string mTitle;
    std::string mArtist;
    std::string mCopyright;
    ChunkType mChunkType;

    FormatError serialize(std::ostream &stream, Song &song);
    FormatError serialize(std::ostream &stream, Instrument &inst);
    FormatError serialize(std::ostream &stream, Waveform &wave);



};



}