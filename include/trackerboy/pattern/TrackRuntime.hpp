
#pragma once

#include "trackerboy/instrument/Instrument.hpp"
#include "trackerboy/instrument/InstrumentRuntime.hpp"
#include "trackerboy/pattern/Track.hpp"


namespace trackerboy {


template <ChType ch>
class TrackRuntime {

public:
    TrackRuntime();

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

    OutputMode mOutputMode;

    uint8_t mLastInstrumentId;
    Instrument *mLastInstrument;
    uint16_t mFreq;

    InstrumentRuntime<ch> mIr;

    // persistent note settings
    uint8_t mEnvelope; // CH1, CH2 envelope register, CH3 = wave id
    uint8_t mTimbre; // CH1, CH2 = duty sequence, CH3 = wave volume


};


}
