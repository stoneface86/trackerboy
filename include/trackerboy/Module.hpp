
#pragma once

#include "trackerboy/Table.hpp"

#include <fstream>
#include <string>


namespace trackerboy {


class Module {

public:
    Module();

    std::string artist();

    std::string title();

    std::string copyright();

    SongTable& songTable();

    WaveTable& waveTable();

    InstrumentTable& instrumentTable();

    void setArtist(std::string artist);

    void setTitle(std::string title);

    void setCopyright(std::string copyright);


private:

    SongTable mSongTable;
    WaveTable mWaveTable;
    InstrumentTable mInstrumentTable;


    // information about the module (same format as *.gbs)
    std::string mTitle;
    std::string mArtist;
    std::string mCopyright;

};


}
