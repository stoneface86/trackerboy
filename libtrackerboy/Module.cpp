
#include "trackerboy/Module.hpp"


namespace trackerboy {


Module::Module() :
    mSongTable(),
    mWaveTable(),
    mInstrumentTable(),
    mTitle(""),
    mArtist(""),
    mCopyright("") 
{
    mTitle.reserve(INFO_STR_SIZE);
    mArtist.reserve(INFO_STR_SIZE);
    mCopyright.reserve(INFO_STR_SIZE);
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
    artist.copy(&mArtist.front(), INFO_STR_SIZE);
}

void Module::setCopyright(std::string copyright) {
    copyright.copy(&mCopyright.front(), INFO_STR_SIZE);
}

void Module::setTitle(std::string title) {
    title.copy(&mTitle.front(), INFO_STR_SIZE);
}

}