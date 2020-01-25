
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

namespace trackerboy {

File::File(std::string path) :
    mRevision(FILE_REVISION),
    mTitle(""),
    mArtist(""),
    mCopyright(""),
    mChunkType(ChunkType::mod),
    mChunkSize(0)
{
}

File::~File() {

}

std::ifstream File::load(std::string path, FormatError &error) {

    Header header;
    std::ifstream input(path, std::ios::binary | std::ios::in);

    if (input.is_open()) {

        // do once loop
        do {
            // read in the header (160 bytes at the start of the file)
            input.read(reinterpret_cast<char*>(&header), sizeof(header));

            if (!input.good()) {
                error = FormatError::readError;
                break;
            }

            // check the signature
            if (!std::equal(header.signature, header.signature + Header::SIGNATURE_SIZE, FILE_MODULE_SIGNATURE)) {
                error = FormatError::invalidSignature;
                break;
            }

            // check revision, do not attempt to parse future versions
            if (header.revision > FILE_REVISION) {
                error = FormatError::invalidRevision;
                break;
            }

            // check chunk type
            if (header.type > static_cast<uint8_t>(ChunkType::last)) {
                error = FormatError::invalidTableCode;
                break;
            }

            // no errors with format
            error = FormatError::none;

            mRevision = header.revision;
            mChunkType = static_cast<ChunkType>(header.type);
            mChunkSize = correctEndian(header.chunkSize);

            // ensure strings are null terminated
            header.title[Header::TITLE_LENGTH - 1] = '\0';
            header.artist[Header::ARTIST_LENGTH - 1] = '\0';
            header.copyright[Header::COPYRIGHT_LENGTH - 1] = '\0';

            mTitle = std::string(header.title);
            mArtist = std::string(header.artist);
            mCopyright = std::string(header.copyright);

        } while (false);

    } else {
        // couldn't open file, just set error
        error = FormatError::readError;
    }

    return input;
}

std::ofstream File::save(std::string path, FormatError &error) {

    std::ofstream output(path, std::ios::out | std::ios::binary);

    if (output.is_open()) {

        // setup the header

        Header header{ 0 };
        
        // signature
        std::copy(FILE_MODULE_SIGNATURE, FILE_MODULE_SIGNATURE + Header::SIGNATURE_SIZE, header.signature);

        // version information (saving always overrides what was loaded)
        header.versionMajor = correctEndian(VERSION_MAJOR);
        header.versionMinor = correctEndian(VERSION_MINOR);
        header.versionPatch = correctEndian(VERSION_PATCH);

        // file information

        // revision remains the same as the one that was loaded.
        // for new files, it is set to the current revision.
        header.revision = mRevision;
        header.type = static_cast<uint8_t>(mChunkType);

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

        header.chunkSize = correctEndian(mChunkSize);

        // write the header
        output.write(reinterpret_cast<char*>(&header), sizeof(header));
        if (output.good()) {
            error = FormatError::none;
        } else {
            error = FormatError::writeError;
        }

    } else {
        // could not open file
        error = FormatError::writeError;
    }

    return output;
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

void File::setChunkSize(uint32_t size) {
    mChunkSize = size;
}

void File::setChunkType(ChunkType type) {
    mChunkType = type;
}

template <class T>
FormatError File::saveTable(std::ofstream &stream, Table<T> &table) {
    if (!stream.good()) {
        return FormatError::writeError;
    }

    // before we do anything, check the file's revision. If it differs from
    // the current one then we must use the older format for
    // backwards-compatibility. New files always use the current revision.

    // since we only have the initial version, we do not need to check
    // anything as forward-compatibilty is not supported.


    // write the size of the table
    uint8_t tableSize = table.size();
    stream.write(reinterpret_cast<char*>(&tableSize), 1);
    
}

// privates

// wrapper for stream.write, will return writeError on failure
#define writeAndCheck(stream, buf, count) \
    do {\
        stream.write(reinterpret_cast<char*>(buf), count); \
        if (!stream.good()) { \
            return FormatError::writeError; \
        } \
    } while (false)

FormatError File::serialize(std::ofstream &stream, Song &song) {
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

    // since this is a private helper method, the stream is assumed to be good
    // before calling this method
    
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
        patternHeader.rows = static_cast<uint8_t>(iter->size());
        writeAndCheck(stream, &patternHeader, sizeof(patternHeader));
        // pattern data
        auto patternEnd = iter->end();
        // write every trackrow in order
        for (auto trackIter = iter->begin(); trackIter != patternEnd; ++trackIter) {
            writeAndCheck(stream, &trackIter, sizeof(*trackIter));
        }
    }
    
    return FormatError::none;
}


}