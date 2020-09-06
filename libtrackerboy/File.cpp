
#include <algorithm>

#include "trackerboy/File.hpp"
#include "trackerboy/data/Table.hpp"

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
    uint8_t orderCount;         // 1-256
    uint16_t rowsPerTrack;      // 1-256
    uint16_t tracks;            // 0-1024
    // order data...
    // pattern data...

};

struct TrackFormat {
    uint8_t channel;
    uint8_t trackId;
    uint8_t rows;      // counter is less one
    // row data follows
};


struct RowFormat {
    uint8_t row;
    trackerboy::TrackRow data;
};

#pragma pack(pop)


}

namespace trackerboy {

File::File() :
    mVersion(VERSION),
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
    header.versionMajor = correctEndian(mVersion.major);
    header.versionMinor = correctEndian(mVersion.minor);
    header.versionPatch = correctEndian(mVersion.patch);

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

FormatError File::loadModule(std::istream &stream, Module &mod) {

    FormatError error;

    error = loadTable(stream, mod.instrumentTable());
    if (error != FormatError::none) {
        return error;
    }

    auto &songs = mod.songs();

    uint8_t songCount;
    readAndCheck(stream, &songCount, 1);
    size_t adjSongCount = static_cast<size_t>(songCount) + 1;
    songs.resize(adjSongCount);
    for (size_t i = 0; i != adjSongCount; ++i) {
        auto &song = songs[i];
        error = deserialize(stream, song);
        if (error != FormatError::none) {
            return error;
        }
    }


    if (error != FormatError::none) {
        return error;
    }

    error = loadTable(stream, mod.waveTable());


    return error;
}

FormatError File::saveModule(std::ostream &stream, Module &mod) {

    FormatError error = mod.instrumentTable().serialize(stream);
    //error = saveTable(stream, mod.instrumentTable());
    if (error != FormatError::none) {
        return error;
    }

    auto &songs = mod.songs();
    uint8_t songCount = static_cast<uint8_t>(songs.size()) - 1;
    writeAndCheck(stream, &songCount, 1);
    for (auto &song : songs) {
        error = serialize(stream, song);
        if (error != FormatError::none) {
            return error;
        }
    }
    

    error = mod.waveTable().serialize(stream);
    //error = saveTable(stream, mod.waveTable());

    return error;
}

template <class T>
FormatError File::loadTable(std::istream &stream, Table<T> &table) {

    // get the size of the table
    uint16_t tableSize;
    readAndCheck(stream, &tableSize, sizeof(tableSize));
    tableSize = correctEndian(tableSize);

    if (tableSize > Table<T>::MAX_SIZE) {
        return FormatError::tableSizeBounds;
    }

    table.clear();
    for (uint16_t i = 0; i != tableSize; ++i) {
        uint8_t id;
        readAndCheck(stream, &id, sizeof(id));
        std::string name;
        std::getline(stream, name, '\0');
        if (!stream.good()) {
            return FormatError::readError;
        }

        T& item = table.insert(id, name);
        FormatError error = deserialize(stream, item);
        if (error != FormatError::none) {
            return error;
        }
    }

    return FormatError::none;
}

template <class T>
FormatError File::saveTable(std::ostream &stream, Table<T> &table) {
    
    // before we do anything, check the file's revision. If it differs from
    // the current one then we must use the older format for
    // backwards-compatibility. New files always use the current revision.

    // since we only have the initial version, we do not need to check
    // anything as forward-compatibilty is not supported.


    // write the size of the table
    uint16_t tableSize = correctEndian(static_cast<uint16_t>(table.size()));
    writeAndCheck(stream, &tableSize, sizeof(tableSize));

    //// write all items in the table in this order: id, name, item
    for (auto id : table) {
        // id
        writeAndCheck(stream, &id, 1);

        T* item = table[id];

        // name
        auto name = item->name();
        writeAndCheck(stream, const_cast<char*>(name.c_str()), name.size() + 1);

        // item
        FormatError error = serialize(stream, *item);
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
    /*auto& program = inst.getProgram();
    program.clear();

    uint8_t size;
    readAndCheck(stream, &size, sizeof(size));

    program.resize(size);
    readAndCheck(stream, program.data(), size);*/

    // just read the entire struct
    auto &data = inst.data();
    readAndCheck(stream, &data, sizeof(Instrument::Data));

    return FormatError::none;
}


FormatError File::deserialize(std::istream &stream, Song &song) {

    // read in the song settings
    SongFormat songHeader;
    readAndCheck(stream, &songHeader, sizeof(songHeader));

    // correct endian if needed
    songHeader.tempo = correctEndian(songHeader.tempo);
    songHeader.tracks = correctEndian(songHeader.tracks);
    songHeader.rowsPerTrack = correctEndian(songHeader.rowsPerTrack);

    song.setTempo(songHeader.tempo);
    song.setRowsPerBeat(songHeader.rowsPerBeat);
    song.setSpeed(songHeader.speed);
    
    auto &orders = song.orders();
    size_t orderCount = static_cast<size_t>(songHeader.orderCount) + 1;
    for (size_t i = 0; i != orderCount; ++i) {
        Order ord;
        readAndCheck(stream, &ord, sizeof(ord));
        orders.push_back(ord);
    }

    auto &pm = song.patterns();
    pm.setRowSize(songHeader.rowsPerTrack);

    for (uint16_t i = 0; i != songHeader.tracks; ++i) {
        TrackFormat trackFormat;
        readAndCheck(stream, &trackFormat, sizeof(TrackFormat));
        if (trackFormat.channel > static_cast<uint8_t>(ChType::ch4)) {
            return FormatError::readError;
        }

        Track &track = pm.getTrack(static_cast<ChType>(trackFormat.channel), trackFormat.trackId);
        size_t rowCount = static_cast<size_t>(trackFormat.rows) + 1;
        for (uint16_t r = 0; r != rowCount; ++r) {
            RowFormat rowFormat;
            readAndCheck(stream, &rowFormat, sizeof(rowFormat));
            // only byte fields in RowFormat and TrackRow so endian correction isn't needed
            track.replace(rowFormat.row, rowFormat.data);
        }
    }

    


    return FormatError::none;
}


FormatError File::deserialize(std::istream &stream, Waveform &wave) {

    readAndCheck(stream, wave.data(), Gbs::WAVE_RAMSIZE);

    return FormatError::none;
}


FormatError File::serialize(std::ostream &stream, Song &song) {
    
    // song settings
    auto startpos = stream.tellp();

    SongFormat songFormat;
    songFormat.tempo = correctEndian(song.tempo());
    songFormat.rowsPerBeat = song.rowsPerBeat();
    songFormat.speed = song.speed();
    
    auto &orders = song.orders();

    // orderCount is a byte so the count has a bias of -1
    // the size of an order can never be 0
    // [0...255] == [1...256]
    songFormat.orderCount = static_cast<uint8_t>(orders.size() - 1);
    songFormat.rowsPerTrack = correctEndian(song.patterns().rowSize());
    songFormat.tracks = 0; // we'll overwrite this later

    writeAndCheck(stream, &songFormat, sizeof(songFormat));
    
    // order data

    for (auto &order : orders) {
        // TODO: Order struct must be packed for this to work on all machines
        writeAndCheck(stream, &order, sizeof(order));
    }

    // track data
    // we go through every track in the song's pattern master
    // only non-empty tracks get saved (tracks with at least 1 row set)

    auto &pm = song.patterns();

    uint16_t trackCounter = 0;

    // iterate all channels
    for (uint8_t ch = 0; ch <= static_cast<uint8_t>(ChType::ch4); ++ch) {
        // track iterators for the current channel
        auto begin = pm.tracksBegin(static_cast<ChType>(ch));
        auto end = pm.tracksEnd(static_cast<ChType>(ch));

        // iterate all tracks for this channel
        for (auto pair = begin; pair != end; ++pair) {
            // make sure the track isn't empty (we only serialize non-empty tracks)
            if (pair->second.rowCount() > 0) {
                // write out the track data
                TrackFormat trackFormat;
                trackFormat.channel = ch;
                trackFormat.trackId = pair->first;
                trackFormat.rows = static_cast<uint8_t>(pair->second.rowCount() - 1);
                writeAndCheck(stream, &trackFormat, sizeof(trackFormat));
                uint8_t rowno = 0;
                for (auto &row : pair->second) {
                    if (row.flags) {
                        // row is non-empty, write out the row
                        RowFormat rowFormat;
                        rowFormat.row = rowno;
                        rowFormat.data = row;
                        writeAndCheck(stream, &rowFormat, sizeof(rowFormat));
                    }

                    ++rowno;
                }

                ++trackCounter;
            }
        }

    }

    // go back to the song format, we need to update the track count
    auto curpos = stream.tellp();
    stream.seekp(static_cast<size_t>(startpos) + offsetof(SongFormat, tracks));
    trackCounter = correctEndian(trackCounter);
    writeAndCheck(stream, &trackCounter, sizeof(trackCounter));

    stream.seekp(curpos);
    
    return FormatError::none;
}

FormatError File::serialize(std::ostream &stream, Instrument &inst) {
    
    //auto &program = inst.getProgram();

    // size of the program
    //uint8_t size = static_cast<uint8_t>(program.size());
    //writeAndCheck(stream, &size, sizeof(size));

    // program is just a byte array, no need to worry about endianness
    //writeAndCheck(stream, program.data(), size);
    
    writeAndCheck(stream, &inst.data(), sizeof(Instrument::Data));

    return FormatError::none;
}

FormatError File::serialize(std::ostream &stream, Waveform &wave) {
    
    writeAndCheck(stream, wave.data(), Gbs::WAVE_RAMSIZE);
    
    return FormatError::none;
}

template FormatError File::loadTable<Instrument>(std::istream &stream, InstrumentTable &table);
template FormatError File::loadTable<Waveform>(std::istream &stream, WaveTable &table);

template FormatError File::saveTable<Instrument>(std::ostream &stream, InstrumentTable &table);
template FormatError File::saveTable<Waveform>(std::ostream &stream, WaveTable &table);



}
