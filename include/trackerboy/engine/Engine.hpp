
#pragma once

#include <cstdint>
#include <optional>
#include <tuple>

#include "trackerboy/engine/Timer.hpp"
#include "trackerboy/engine/PatternCursor.hpp"
#include "trackerboy/engine/PatternRuntime.hpp"
#include "trackerboy/song/Song.hpp"

namespace trackerboy {


class Engine {

public:

    Engine();

    void reset();

    void play(Song &song, uint8_t orderNo, uint8_t patternRow = 0);
    
    bool step(Synth &synth, InstrumentTable &itable, WaveTable &wtable);

private:

    template <int channel = 0>
    bool setRows();

    enum class PatternCommand {
        none,
        next,
        jump
    };

    struct Context {
        Context(Song &song, uint8_t orderNo, uint8_t lastOrder);

        Song &song;
        uint8_t orderCounter;
        // last order index for the song
        uint8_t const lastOrder;

        
        bool halted;
        PatternCommand command;
        uint8_t commandParam;
    };

    
    //friend std::optional<Context>;
    std::optional<Context> mContext;

    // Runtime components

    Timer mTimer;
    PatternCursor mCursor;

    std::tuple<
        TrackRuntime<ChType::ch1>,
        TrackRuntime<ChType::ch2>,
        TrackRuntime<ChType::ch3>,
        TrackRuntime<ChType::ch4>
    > mTrTuple;

    /*TrackRuntime<ChType::ch1> mTr1;
    TrackRuntime<ChType::ch2> mTr2;
    TrackRuntime<ChType::ch3> mTr3;
    TrackRuntime<ChType::ch4> mTr4;*/

};


}
