
#include <algorithm>

#include "version.hpp"

#include "trackerboy/File.hpp"
#include "trackerboy/Table.hpp"

// NOTE: Trackerboy file revision will remain at 0 until changes are made
// to the file format AFTER release (>= v1.0.0). The file format is subject
// to changes AT ANY MOMENT during development. When the structure or protocol
// of the file format changes after release, the FILE_REVISION is incremented
// by 1 and new files will be saved with this revision (older files can use
// the new revision by calling upgrade(), if possible).

// When the file revision changes, support for backwards compatibilty must be
// implemented. Affected parts of the new format should check the file's
// revision before saving/loading with the new format. By default, the current
// revision is used for new files.

#define readAndCheck(stream, buf, count) \
    do { \
        stream.read(reinterpret_cast<char*>(buf), count); \
        if (!stream.good()) { \
            return FormatError::readError; \
        } \
    } while (false)


// wrapper for stream.write, will return writeError on failure
#define writeAndCheck(stream, buf, count) \
    do {\
        stream.write(reinterpret_cast<char*>(buf), count); \
        if (!stream.good()) { \
            return FormatError::writeError; \
        } \
    } while (false)

namespace {

#pragma pack(push, 1)

struct SongFormat {
    float tempo;
    uint8_t rowsPerBeat;
    uint8_t speed;
    uint8_t numberOfPatterns;
    // order data...
    // pattern data...

};

struct OrderFormat {
    uint8_t loopFlag;
    uint8_t loopIndex;
    uint8_t orderListSize;
    // order list data
};

struct PatternFormat {
    uint8_t rows;
    // row data follows
};

#pragma pack(pop)

}

namespace trackerboy {

File::File() :
    mRevision(FILE_REVISION),
    mTitle(""),
    mArtist(""),
    mCopyright(""),
    mFileType(FileType::mod)
{
}

File::~File() {

}

FormatError File::loadHeader(std::istream &stream) {

    // read in the header
    Header header;
    readAndCheck(stream, &header, sizeof(header));

    // check the signature
    if (!std::equal(header.signature, header.signature + Header::SIGNATURE_SIZE, FILE_SIGNATURE)) {
        return FormatError::invalidSignature;
    }

    // check revision, do not attempt to parse future versions
    if (header.revision > FILE_REVISION) {
        return FormatError::invalidRevision;
    }

    // check file type
    if (header.type > static_cast<uint8_t>(FileType::last)) {
        return FormatError::invalidType;
    }

    mRevision = header.revision;
    mFileType = static_cast<FileType>(header.type);

    // ensure strings are null terminated
    header.title[Header::TITLE_LENGTH - 1] = '\0';
    header.artist[Header::ARTIST_LENGTH - 1] = '\0';
    header.copyright[Header::COPYRIGHT_LENGTH - 1] = '\0';

    mTitle = std::string(header.title);
    mArtist = std::string(header.artist);
    mCopyright = std::string(header.copyright);

    return FormatError::none;
}

FormatError File::saveHeader(std::ostream &stream) {

    // setup the header

    Header header{ 0 };
        
    // signature
    std::copy(FILE_SIGNATURE, FILE_SIGNATURE + Header::SIGNATURE_SIZE, header.signature);

    // version information (saving always overrides what was loaded)
    header.versionMajor = correctEndian(VERSION_MAJOR);
    header.versionMinor = correctEndian(VERSION_MINOR);
    header.versionPatch = correctEndian(VERSION_PATCH);

    // file information

    // revision remains the same as the one that was loaded.
    // for new files, it is set to the current revision.
    header.revision = mRevision;
    header.type = static_cast<uint8_t>(mFileType);

    #define copyStringToFixed(dest, string, count) do { \
            size_t len = std::min(count - 1, string.length()); \
            string.copy(dest, len); \
            dest[len] = '\0'; \
        } while (false)

    copyStringToFixed(header.title, mTitle, Header::TITLE_LENGTH);
    copyStringToFixed(header.artist, mArtist, Header::ARTIST_LENGTH);
    copyStringToFixed(header.copyright, mCopyright, Header::COPYRIGHT_LENGTH);

    #undef copyStringToFixed
        
    // reserved was zero'd on initialization of header

    // write the header
    writeAndCheck(stream, &header, sizeof(header));

    return FormatError::none;

}

void File::setArtist(std::string artist) {
    mArtist = artist;
}

void File::setCopyright(std::string copyright) {
    mCopyright = copyright;
}

void File::setTitle(std::string title) {
    mTitle = title;
}

void File::setFileType(FileType type) {
    mFileType = type;
}

template <class T>
FormatError File::saveTable(std::ostream &stream, Table<T> &table) {
    
    // before we do anything, check the file's revision. If it differs from
    // the current one then we must use the older format for
    // backwards-compatibility. New files always use the current revision.

    // since we only have the initial version, we do not need to check
    // anything as forward-compatibilty is not supported.


    // write the size of the table
    uint8_t tableSize = static_cast<uint8_t>(table.size());
    writeAndCheck(stream, &tableSize, sizeof(tableSize));

    // write all items in the table in this order: id, name, item
    for (auto iter : table) {
        // id
        uint8_t id = iter.first;
        writeAndCheck(stream, &id, 1);
        // name
        writeAndCheck(stream, iter.second.name.front(), iter.second.name.size());

        // item
        FormatError error = serialize(stream, iter.second.value);
        if (error != FormatError::none) {
            return error;
        }
    }

    return FormatError::none;
    
}

uint8_t File::revision() {
    return mRevision;
}

std::string File::artist() {
    return mArtist;
}

std::string File::copyright() {
    return mCopyright;
}

std::string File::title() {
    return mTitle;
}

FileType File::fileType() {
    return mFileType;
}

FormatError File::deserialize(std::istream &stream, Instrument &inst) {
    auto& program = inst.getProgram();
    program.clear();

    uint8_t size;
    readAndCheck(stream, &size, sizeof(size));

    program.resize(size);
    readAndCheck(stream, program.data(), sizeof(Instruction) * size);

    return FormatError::none;
}


FormatError File::deserialize(std::istream &stream, Song &song) {

    // read in the song settings
    SongFormat songHeader;
    readAndCheck(stream, &songHeader, sizeof(songHeader));

    // correct endian if needed
    songHeader.tempo = correctEndian(songHeader.tempo);

    song.setTempo(songHeader.tempo);
    song.setRowsPerBeat(songHeader.rowsPerBeat);
    song.setSpeed(songHeader.speed);
    
    // read in the order
    {
        OrderFormat orderHeader;
        readAndCheck(stream, &orderHeader, sizeof(orderHeader));
        auto &order = song.order();
        auto &orderVec = order.indexVec();
        orderVec.clear();
        orderVec.resize(orderHeader.orderListSize);
        readAndCheck(stream, orderVec.data(), orderHeader.orderListSize);
        
        if (orderHeader.loopFlag) {
            order.setLoop(orderHeader.loopIndex);
        } else {
            order.removeLoop();
        }
    }

    // read in the patterns
    
    auto &patterns = song.patterns();
    patterns.clear();
    for (uint8_t i = 0; i != songHeader.numberOfPatterns; ++i) {
        // first byte is the size of the pattern
        PatternFormat patternHeader;
        readAndCheck(stream, &patternHeader, sizeof(patternHeader));

        // pattern size ranges from 1-256, so add one to what we read in
        size_t rows = static_cast<size_t>(patternHeader.rows) + 1;
        Pattern p(rows);
        readAndCheck(stream, p.data(), sizeof(TrackRow) * 4 * rows);

        patterns.push_back(p);
        
    }


    


    return FormatError::none;
}


FormatError File::deserialize(std::istream &stream, Waveform &wave) {

    readAndCheck(stream, wave.data(), Gbs::WAVE_RAMSIZE);

    return FormatError::none;
}


FormatError File::serialize(std::ostream &stream, Song &song) {
    
    // song settings

    SongFormat songHeader;
    songHeader.tempo = correctEndian(song.actualTempo());
    songHeader.rowsPerBeat = song.rowsPerBeat();
    songHeader.speed = song.speed();
    songHeader.numberOfPatterns = static_cast<uint8_t>(song.patterns().size());

    writeAndCheck(stream, &songHeader, sizeof(songHeader));
    
    // order settings

    auto &order = song.order();
    OrderFormat orderHeader;
    orderHeader.loopFlag = static_cast<uint8_t>(order.loops());
    orderHeader.loopIndex = order.loopIndex();
    orderHeader.orderListSize = static_cast<uint8_t>(order.indexVec().size());

    writeAndCheck(stream, &orderHeader, sizeof(orderHeader));
    
    // order data
    writeAndCheck(stream, order.indexVec().data(), orderHeader.orderListSize);

    // patterns

    auto &patterns = song.patterns();
    auto patternListEnd = patterns.end();
    for (auto iter = patterns.begin(); iter != patternListEnd; ++iter) {
        // pattern settings (just the size)
        PatternFormat patternHeader;
        patternHeader.rows = static_cast<uint8_t>(iter->size() - 1);
        writeAndCheck(stream, &patternHeader, sizeof(patternHeader));
        // pattern data
        writeAndCheck(stream, iter->data(), sizeof(TrackRow) * 4 * iter->size());
    }
    
    return FormatError::none;
}

FormatError File::serialize(std::ostream &stream, Instrument &inst) {
    
    auto &program = inst.getProgram();

    // size of the program
    uint8_t size = static_cast<uint8_t>(program.size());
    writeAndCheck(stream, &size, sizeof(size));

    // since the Instruction structure only has byte fields, we don't need to
    // worry about endianness, so just write the entire program
    writeAndCheck(stream, program.data(), size * sizeof(Instruction));
    
    return FormatError::none;
}

FormatError File::serialize(std::ostream &stream, Waveform &wave) {
    
    writeAndCheck(stream, wave.data(), Gbs::WAVE_RAMSIZE);
    
    return FormatError::none;
}

template FormatError File::saveTable<Instrument>(std::ostream &stream, InstrumentTable &table);
template FormatError File::saveTable<Song>(std::ostream &stream, SongTable &table);
template FormatError File::saveTable<Waveform>(std::ostream &stream, WaveTable &table);



}