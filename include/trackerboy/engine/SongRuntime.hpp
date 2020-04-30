
#pragma once

#include <cstdint>

#include "trackerboy/engine/PatternRuntime.hpp"
#include "trackerboy/song/Song.hpp"

namespace trackerboy {


class SongRuntime {

public:

    static constexpr size_t LOOP_INFINITE = 0;

    SongRuntime(Song &song, uint8_t start);

    bool step(Synth &synth, InstrumentTable &itable, WaveTable &wtable);

private:


    Song &mSong;
    PatternRuntime mPr;

    bool mLoadPattern;

    uint8_t mOc;        // Order counter

};


}
