
#pragma once

#include <cstdint>

#include "trackerboy/pattern/PatternRuntime.hpp"
#include "trackerboy/song/Song.hpp"

namespace trackerboy {


class SongRuntime {

public:

    static constexpr size_t LOOP_INFINITE = 0;

    SongRuntime();

    // songs loops through order for a given number of times (0 for infinite)
    void play(size_t loopCount = LOOP_INFINITE);

    // song plays through order once
    void playOnce();

    void reset();

    void setSong(Song *song);

    bool step(Synth &synth, InstrumentTable &itable, WaveTable &wtable);

private:


    Song *mSong;
    PatternRuntime mPr;
    bool mPlaying;
    Order::Sequencer mSequencer;

    bool mLoadPattern;

};


}
