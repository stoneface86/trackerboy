
#pragma once

#include <cstdint>
#include <optional>

#include "trackerboy/engine/PatternRuntime.hpp"
#include "trackerboy/song/Song.hpp"

namespace trackerboy {


class Engine {

public:

    Engine();

    void play(Song &song, uint8_t orderNo, uint8_t patternRow = 0);
    
    bool step(Synth &synth, InstrumentTable &itable, WaveTable &wtable);

private:

    Song *mSong;
    uint8_t mOc;        // Order counter

    // PatternRuntime is an optional for lazy loading
    // the runtime gets initialized when play is called
    std::optional<PatternRuntime> mPr;

    

};


}
