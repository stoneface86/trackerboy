
#include <algorithm>

#include "version.hpp"

#include "trackerboy/File.hpp"

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
        header.versionMajor = VERSION_MAJOR;
        header.versionMinor = VERSION_MINOR;
        header.versionPatch = VERSION_PATCH;

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

        header.chunkSize = mChunkSize;

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


}