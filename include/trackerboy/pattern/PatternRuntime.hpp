
#pragma once

#include <cstdint>

#include "trackerboy/Q53.hpp"
#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/pattern/TrackRuntime.hpp"


namespace trackerboy {


class PatternRuntime {

public:

    static constexpr Q53 DEFAULT_SPEED = Q53_make(6, 0); // 4 rows/beat @ 150 bpm

    PatternRuntime(uint8_t speed = DEFAULT_SPEED);

    //
    // Reset the runtime to initial state. The pattern will run at the
    // beginning, if one was set.
    //
    void reset();

    //
    // Sets the pattern to run and resets the runtime
    //
    void setPattern(Pattern *pattern);

    //
    // Sets the speed, or frames per row, of the runtime. Speed should be
    // in Q5.3 format (fixed point) and must be in the range 1.0-31.0
    //
    void setSpeed(Q53 speed);

    //
    // Run one frame of the current row from the pattern. If the pattern
    // should stop playing, true is returned.
    //
    bool step(Synth &synth, InstrumentTable &itable, WaveTable &wt);


private:
    Pattern *mPattern;
    Q53 mSpeed;         // speed (frames per row) (Q5.3 format)
    Q53 mFc;            // frame counter

    Pattern::Iterator mIter;
    Pattern::Iterator mEnd;
    
    TrackRuntime mTr1;
    TrackRuntime mTr2;
    TrackRuntime mTr3;
    TrackRuntime mTr4;

    // called when a new pattern is set or reset is called
    void init();

};


}
