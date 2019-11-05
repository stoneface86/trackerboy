
#pragma once

#include <cstdint>

#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/pattern/TrackRow.hpp"
#include "trackerboy/instrument/InstrumentRuntime.hpp"

namespace trackerboy {

enum class PatternStepResult {
    InProgress,     // row is currently being played
    Done,           // row completed normally, next step will start the next row
    PatternEnd,     // no more rows after this one, load next pattern
    PatternSkip     // skip to next pattern and play the next row immediately
};


class PatternRuntime {

public:

    PatternRuntime(ChType trackId);

    //
    // Sets the runtime to play the next row. Returns true if the next
    // row exists, false otherwise.
    //
    bool nextRow();

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
    // Run one frame of the current row from the pattern. If the pattern
    // should stop playing, true is returned.
    //
    bool step(Synth &synth, InstrumentTable &itable, WaveTable &wt);


private:
    Pattern *mPattern;
    InstrumentRuntime mIr;
    const ChType mTrackId;

    Pattern::TrackIterator mTrack;
    Pattern::TrackIterator mTrackEnd;
    TrackRow mCurrentRow;
    bool mIsNewRow;

    bool mIsPlaying;
    uint8_t mLastInstrumentId;
    Instrument *mLastInstrument;
    uint16_t mFreq;

    // effects: TODO

};


}