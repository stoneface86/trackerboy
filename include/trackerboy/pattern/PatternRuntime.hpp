
#pragma once

#include <cstdint>

#include "trackerboy/Q53.hpp"
#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/pattern/TrackRuntime.hpp"


namespace trackerboy {


class PatternRuntime {

public:

    PatternRuntime(Pattern pattern, Q53 speed);

    //
    // Reset the runtime to initial state. The pattern will run at the
    // beginning, if one was set.
    //
    void reset();

    //
    // Sets the pattern to run and resets the runtime
    //
    void setPattern(Pattern pattern);

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
    Pattern mPattern;
    Q53 mSpeed;         // speed (frames per row) (Q5.3 format)
    Q53 mFc;            // frame counter

    TrackData::iterator mIter1;
    TrackData::iterator mIter2;
    TrackData::iterator mIter3;
    TrackData::iterator mIter4;

    // all tracks are the same length so we only need the end iterator
    // for 1
    TrackData::iterator mEnd1;
    
    TrackRuntime<ChType::ch1> mTr1;
    TrackRuntime<ChType::ch2> mTr2;
    TrackRuntime<ChType::ch3> mTr3;
    TrackRuntime<ChType::ch4> mTr4;

    // called when a new pattern is set or reset is called
    void init();

};


}
