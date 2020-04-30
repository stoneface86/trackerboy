
#pragma once

#include <cstdint>
#include <queue>

#include "trackerboy/Speed.hpp"
#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/engine/TrackRuntime.hpp"


namespace trackerboy {


class PatternRuntime {

public:

    enum class State {
        ready,      // ready to step the pattern
        next,       // end of pattern reached: load the next pattern in order
        jump,       // pattern effect: jump to new pattern
        halt        // pattern effect: stop playback completely
    };

    PatternRuntime(Pattern &&pattern, Speed speed, uint8_t lastOrder);

    //
    // Sets the pattern to run from the beginning, the runtime is now ready to
    // step after calling this method
    //
    void setPattern(Pattern &&pattern);

    //
    // Sets the speed, or frames per row, of the runtime. Speed should be
    // in Q5.3 format (fixed point) and must be in the range 1.0-31.0
    // TODO: this method may not be needed as the speed is only set
    //       in the constructor
    //
    void setSpeed(Speed speed);

    State status();

    //
    // if status is jump, this method will return the pattern to jump to
    //
    uint8_t jumpPattern();

    //
    // Run one frame of the current row from the pattern. If the pattern
    // should stop playing, true is returned.
    //
    void step(Synth &synth, InstrumentTable &itable, WaveTable &wt);


private:
    Speed mSpeed;         // speed (frames per row) (Q5.3 format)
    Speed mFc;            // frame counter
    uint8_t const mLastOrder; // index of the last order for the song

    Track::Data::iterator mIter1;
    Track::Data::iterator mIter2;
    Track::Data::iterator mIter3;
    Track::Data::iterator mIter4;

    // all tracks are the same length so we only need the end iterator
    // for 1
    Track::Data::iterator mEnd1;
    
    TrackRuntime<ChType::ch1> mTr1;
    TrackRuntime<ChType::ch2> mTr2;
    TrackRuntime<ChType::ch3> mTr3;
    TrackRuntime<ChType::ch4> mTr4;

    State mStatus;
    State mNextStatus; // status to set at end of row
    uint8_t mJumpTo;

    void parseEffects(TrackRow &row);

};


}
