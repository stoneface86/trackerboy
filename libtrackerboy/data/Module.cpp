
#include "trackerboy/data/Module.hpp"
#include "trackerboy/fileformat.hpp"

#include "./checkedstream.hpp"

#include <algorithm>
#include <cstddef>


namespace trackerboy {


Module::Module() noexcept :
    mSongs(),
    mWaveTable(),
    mInstrumentTable(),
    mVersion(VERSION),
    mRevision(FILE_REVISION),
    mTitle(""),
    mArtist(""),
    mCopyright("") 
{
}

Module::~Module() noexcept {
}

void Module::clear() noexcept {
    mVersion = VERSION;
    mRevision = FILE_REVISION;
    mTitle = "";
    mArtist = "";
    mCopyright = "";
    mSongs.clear();
    mInstrumentTable.clear();
    mWaveTable.clear();
}

std::string Module::artist() const noexcept {
    return mArtist;
}

std::string Module::title() const noexcept {
    return mTitle;
}

std::string Module::copyright() const noexcept {
    return mCopyright;
}

Version Module::version() const noexcept {
    return mVersion;
}

uint8_t Module::revision() const noexcept {
    return mRevision;
}

std::vector<Song>& Module::songs() noexcept {
    return mSongs;
}

InstrumentTable& Module::instrumentTable() noexcept {
    return mInstrumentTable;
}

WaveTable& Module::waveTable() noexcept {
    return mWaveTable;
}

void Module::setArtist(std::string artist) noexcept {
    mArtist = artist;
}

void Module::setCopyright(std::string copyright) noexcept {
    mCopyright = copyright;
}

void Module::setTitle(std::string title) noexcept {
    mTitle = title;
}

// ---- Serialization ----

FormatError Module::deserialize(std::istream &stream) noexcept {

    // read in the header
    Header header;
    checkedRead(stream, &header, sizeof(header));

    // check the signature
    if (!std::equal(header.signature, header.signature + Header::SIGNATURE_SIZE, FILE_SIGNATURE)) {
        return FormatError::invalidSignature;
    }

    // check revision, do not attempt to parse future versions
    if (header.revision > FILE_REVISION) {
        return FormatError::invalidRevision;
    }

    // check file type for module
    if (static_cast<FileType>(header.type) != FileType::mod) {
        return FormatError::invalidType;
    }

    mRevision = header.revision;

    // ensure strings are null terminated
    header.title[Header::TITLE_LENGTH - 1] = '\0';
    header.artist[Header::ARTIST_LENGTH - 1] = '\0';
    header.copyright[Header::COPYRIGHT_LENGTH - 1] = '\0';

    mTitle = std::string(header.title);
    mArtist = std::string(header.artist);
    mCopyright = std::string(header.copyright);

    FormatError error;

    // song list
    uint8_t songCount;
    checkedRead(stream, &songCount, 1);
    size_t adjSongCount = static_cast<size_t>(songCount) + 1;
    mSongs.clear();
    mSongs.resize(adjSongCount);
    for (size_t i = 0; i != adjSongCount; ++i) {
        auto &song = mSongs[i];
        error = song.deserialize(stream);
        if (error != FormatError::none) {
            return error;
        }
    }

    // instrument table
    error = mInstrumentTable.deserialize(stream);
    if (error != FormatError::none) {
        return error;
    }

    // wave table
    error = mWaveTable.deserialize(stream);


    return error;
}

FormatError Module::serialize(std::ostream &stream) noexcept {
    

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
    header.type = static_cast<uint8_t>(FileType::mod);

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
    checkedWrite(stream, &header, sizeof(header));
    
    FormatError error;

    // write songs
    uint8_t songCount = static_cast<uint8_t>(mSongs.size()) - 1;
    checkedWrite(stream, &songCount, sizeof(songCount));
    for (auto &song : mSongs) {
        error = song.serialize(stream);
        if (error != FormatError::none) {
            return error;
        }
    }

    // instrument table
    error = mInstrumentTable.serialize(stream);
    if (error != FormatError::none) {
        return error;
    }

    // wave table
    return mWaveTable.serialize(stream);
}

}
