
#include "trackerboy/Module.hpp"
#include "trackerboy/fileformat.hpp"

#include "version.hpp"

#include <algorithm>
#include <cstddef>


namespace trackerboy {


Module::Module() :
    mSongTable(),
    mWaveTable(),
    mInstrumentTable(),
    mTitle(""),
    mArtist(""),
    mCopyright("") 
{
}

std::string Module::artist() {
    return mArtist;
}

std::string Module::title() {
    return mTitle;
}

std::string Module::copyright() {
    return mCopyright;
}

SongTable& Module::songTable() {
    return mSongTable;
}

InstrumentTable& Module::instrumentTable() {
    return mInstrumentTable;
}

WaveTable& Module::waveTable() {
    return mWaveTable;
}

void Module::serialize(std::ofstream &stream) {
    ModuleHeader header;
    std::copy_n(FILE_MODULE_SIGNATURE, 12, header.signature);
    // version string
    std::string version = "v" VERSION_STR;
    version.resize(sizeof(header.version));
    version.copy(header.version, sizeof(header.version));

    header.revision = FILE_REVISION;
    auto chars = mTitle.copy(header.title, sizeof(header.title));
    if (chars < sizeof(header.title)) {
        std::fill_n(header.title + chars, sizeof(header.title) - chars, '\0');
    }
    chars = mArtist.copy(header.artist, sizeof(header.artist));
    if (chars < sizeof(header.artist)) {
        std::fill_n(header.artist + chars, sizeof(header.artist) - chars, '\0');
    }
    chars = mCopyright.copy(header.copyright, sizeof(header.copyright));
    if (chars < sizeof(header.copyright)) {
        std::fill_n(header.copyright + chars, sizeof(header.copyright) - chars, '\0');
    }

    auto pos = stream.tellp();

    header.instrumentTableOffset = correctEndian(static_cast<uint8_t>(sizeof(header) + pos));
    
    // ignore the next two offsets for now
    auto stoffpos = offsetof(ModuleHeader, songTableOffset) + pos;
    
    // write the header
    stream.write(reinterpret_cast<const char *>(&header), sizeof(header));

    // serialize the instrument table
    mInstrumentTable.serialize(stream);
    uint32_t stoff = correctEndian(static_cast<uint32_t>(stream.tellp()));

    // serialize the song table
    mSongTable.serialize(stream);
    uint32_t wtoff = correctEndian(static_cast<uint32_t>(stream.tellp()));

    // serialize the wave table
    mWaveTable.serialize(stream);
    pos = stream.tellp();

    // go back to the offsets in the header we skipped
    stream.seekp(stoffpos);
    stream.write(reinterpret_cast<const char *>(&stoff), 4);
    stream.write(reinterpret_cast<const char *>(&wtoff), 4);

    // terminate
    stream.seekp(pos);
    stream.write(FILE_TERMINATOR, 3);


}

void Module::setArtist(std::string artist) {
    mArtist = artist;
}

void Module::setCopyright(std::string copyright) {
    mCopyright = copyright;
}

void Module::setTitle(std::string title) {
    mTitle = title;
}

}