
#pragma once

#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/engine/IApu.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"


#include <optional>

namespace trackerboy {

//
// Utility class for previewing instruments.
//
class InstrumentPreview {

public:

    InstrumentPreview();

    void setInstrument(std::shared_ptr<Instrument> instrument);

    void play(uint8_t note);

    void step(RuntimeContext const& rc);

private:

    void restart();

    ChType mCh;
    ToneFrequencyControl mToneFc;
    NoiseFrequencyControl mNoiseFc;
    FrequencyControl *mFc;

    std::shared_ptr<Instrument> mInstrument;
    std::optional<InstrumentRuntime> mIr;

    bool mInit;
    bool mRetrigger;
    ChannelState mLastState;
};


}
