
#pragma once

#include "trackerboy/instrument/Instrument.hpp"
#include "trackerboy/instrument/InstrumentRuntime.hpp"
#include "trackerboy/pattern/Track.hpp"


namespace trackerboy {


class TrackRuntime {


public:

    TrackRuntime(ChType trackId);


    //
    // Reset the runtime to initial state. The pattern will run at the
    // beginning, if one was set.
    //
    void reset();

    //
    // Sets the track to run
    //
    void setRow(TrackRow row, InstrumentTable &table);

    //
    // Run one frame of the current row from the track. If the track
    // should stop playing, true is returned.
    //
    void step(Synth &synth, WaveTable &wt);


private:

    enum class OutputMode {
        STOPPED_DISABLE_MIXER,
        STOPPED,
        PLAYING_ENABLE_MIXER,
        PLAYING
    };

    const ChType mTrackId;
    InstrumentRuntime mIr;

    OutputMode mOutputMode;

    uint8_t mLastInstrumentId;
    Instrument *mLastInstrument;
    uint16_t mFreq;


};


}
