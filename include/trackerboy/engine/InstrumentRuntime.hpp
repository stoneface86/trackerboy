
#pragma once

#include "trackerboy/trackerboy.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/TrackRow.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/Operation.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"
#include "trackerboy/engine/ChannelState.hpp"

namespace trackerboy {

class InstrumentRuntime {


public:
    InstrumentRuntime(FrequencyControl &fc);

    void restart();

    void setInstrument(std::shared_ptr<Instrument> instrument);

    void step(ChannelState &state);


private:

    FrequencyControl &mFc;

    std::shared_ptr<Instrument> mInstrument;

    bool mRestart;

    Sequence::Enumerator mPanningSequence;
    Sequence::Enumerator mTimbreSequence;

};


}
