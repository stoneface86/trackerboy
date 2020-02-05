
#pragma once


#include "trackerboy/fileformat.hpp"
#include "trackerboy/Table.hpp"
#include "trackerboy/Module.hpp"
#include "trackerboy/version.hpp"


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

    FormatError loadModule(std::istream &stream, Module &mod);

    FormatError saveModule(std::ostream &stream, Module &mod);

    template <class T>
    FormatError loadTable(std::istream &stream, Table<T> &table);
    
    template <class T>
    FormatError saveTable(std::ostream &stream, Table<T> &table);

    // upgrades a loaded file from an older revision to the current
    //void upgrade();

    void setArtist(std::string artist);

    void setCopyright(std::string copyright);

    void setTitle(std::string title);

    void setFileType(FileType type);

    void setVersion(Version version);

    uint8_t revision();

    std::string title();
    
    std::string artist();

    std::string copyright();

    FileType fileType();

    Version version();

    FormatError deserialize(std::istream &stream, Instrument &inst);
    FormatError deserialize(std::istream &stream, Song &song);
    FormatError deserialize(std::istream &stream, Waveform &wave);


    // serialization methods

    FormatError serialize(std::ostream &stream, Song &song);
    FormatError serialize(std::ostream &stream, Instrument &inst);
    FormatError serialize(std::ostream &stream, Waveform &wave);

private:
    // header settings
    Version mVersion;
    uint8_t mRevision;
    std::string mTitle;
    std::string mArtist;
    std::string mCopyright;
    FileType mFileType;

    



};



}
