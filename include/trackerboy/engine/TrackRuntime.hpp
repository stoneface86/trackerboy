
#pragma once

#include "trackerboy/trackerboy.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/TrackRow.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"

#include <array>
#include <type_traits>

namespace trackerboy {

class TrackRuntimeBase {

public:
    void setRow(RuntimeContext const& rc, TrackRow const& row);

    void step(RuntimeContext const& rc);

protected:
    TrackRuntimeBase(ChType ch, FrequencyControl &fc);

private:
    enum class State {
        delayed,
        playing,
        stopped


    };

    void handleRow(RuntimeContext const& rc);

    void noteCut(RuntimeContext const& rc);


    ChType const mCh;
    uint8_t const mRegAddr;
    FrequencyControl &mFc;

    State mState;
    bool mDelayingRow;
    bool mNewRow;
    bool mIsPlaying;

    TrackRow mCurrentRow;

    Instrument *mInstrument;

    bool mLocked;

    uint8_t mDelayCounter;
    uint8_t mCutCounter;

    Sequence::Enumerator mPanningSequence;
    Sequence::Enumerator mTimbreSequence;


    // set by Exx
    uint8_t mEnvelope;
    // set by Vxx
    uint8_t mTimbre;
    // set by Ixy
    Panning mPanning;

    // shadow registers
    // a copy of the channel's registers, we write to these first and
    // then the channel's registers if the channel is locked
    std::array<uint8_t, GB_CHANNEL_REGS> mShadow;

};

template <ChType ch>
class TrackRuntime : public TrackRuntimeBase {

public:
    TrackRuntime();

private:
    // Channels 1, 2 and 3 get ToneFrequencyControl, channel 4 gets NoiseFrequencyControl
    using FcClass = std::conditional_t<ch == ChType::ch4, NoiseFrequencyControl, ToneFrequencyControl>;

    FcClass mFcImpl;

};


}
