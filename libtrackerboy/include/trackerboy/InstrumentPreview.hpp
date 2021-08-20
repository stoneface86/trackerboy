
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

    //
    // Start previewing the given instrument. If instrument is nullptr, then
    // the runtime will preview the set note on the given channel. If no channel
    // is provided, then it will default to the instrument's channel or CH1 if
    // no instrument was provided.
    //
    void setInstrument(std::shared_ptr<Instrument> instrument, std::optional<ChType> ch = std::nullopt);

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
