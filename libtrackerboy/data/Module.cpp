
#include "trackerboy/data/Module.hpp"
#include "trackerboy/fileformat.hpp"

#include <algorithm>
#include <cstddef>


namespace trackerboy {


Module::Module() :
    mSongs(),
    mWaveTable(),
    mInstrumentTable(),
    mTitle(""),
    mArtist(""),
    mCopyright("") 
{
}

void Module::clear() {
    mTitle = "";
    mArtist = "";
    mCopyright = "";
    mSongs.clear();
    mInstrumentTable.clear();
    mWaveTable.clear();
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

//SongTable& Module::songTable() {
//    return mSongTable;
//}

std::vector<Song>& Module::songs() {
    return mSongs;
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
