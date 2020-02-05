
#include "trackerboy/Module.hpp"
#include "trackerboy/fileformat.hpp"

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
