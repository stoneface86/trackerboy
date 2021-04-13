
#pragma once

#include "trackerboy/trackerboy.hpp"
#include "trackerboy/data/TrackRow.hpp"
#include "trackerboy/engine/Operation.hpp"
#include "trackerboy/engine/ChannelState.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/GlobalState.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"


namespace trackerboy {

class TrackControl {

public:
    TrackControl(ChType ch, FrequencyControl &fc);

    void setRow(TrackRow const& row);

    void step(RuntimeContext const& rc, ChannelState &state, GlobalState &global);

private:


    Operation mOp;

    std::shared_ptr<Instrument> mInstrument;
    FrequencyControl &mFc;
    std::optional<InstrumentRuntime> mIr;

    std::optional<uint8_t> mDelayCounter;
    std::optional<uint8_t> mCutCounter;

    bool mPlaying;

    uint8_t mEnvelope;
    uint8_t mPanning;
    uint8_t mTimbre;


};

class ToneTrackControl : public TrackControl {

public:
    ToneTrackControl(ChType ch);

private:
    ToneFrequencyControl mFc;


};

class NoiseTrackControl : public TrackControl {

public:
    NoiseTrackControl();

private:
    NoiseFrequencyControl mFc;


};


}
