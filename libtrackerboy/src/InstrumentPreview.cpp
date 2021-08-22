
#include "trackerboy/InstrumentPreview.hpp"

#include "trackerboy/engine/ChannelControl.hpp"

namespace trackerboy {


InstrumentPreview::InstrumentPreview() :
    mCh(ChType::ch1),
    mToneFc(),
    mNoiseFc(),
    mFc(&mToneFc),
    mInstrument(),
    mIr(),
    mInit(false),
    mRetrigger(false),
    mLastState()
{
}

void InstrumentPreview::setInstrument(std::shared_ptr<Instrument> instrument, std::optional<ChType> ch) {
    // ensure that the previous instrument is no longer used anywhere else after
    // hitting this assignment.
    mInstrument = std::move(instrument);
    if (mInstrument) {
        mCh = ch.value_or(mInstrument->channel());
    } else {
        mCh = ch.value_or(ChType::ch1);
        mIr.reset();
    }

    if (mCh == ChType::ch4) {
        mFc = &mNoiseFc;
    } else {
        mFc = &mToneFc;
    }

    restart();
    mInit = true;
}


void InstrumentPreview::play(uint8_t note) {
    Operation op;
    op.note = note;
    mFc->apply(op);
    restart();
}

void InstrumentPreview::step(RuntimeContext const& rc) {

    ChannelState state;
    
    if (mInit) {
        mLastState = ChannelState(mCh);
        switch (mCh) {
            case ChType::ch1:
                ChannelControl<ChType::ch1>::init(rc.apu, rc.waveTable, mLastState);
                break;
            case ChType::ch2:
                ChannelControl<ChType::ch2>::init(rc.apu, rc.waveTable, mLastState);
                break;
            case ChType::ch3:
                ChannelControl<ChType::ch3>::init(rc.apu, rc.waveTable, mLastState);
                break;
            case ChType::ch4:
                ChannelControl<ChType::ch4>::init(rc.apu, rc.waveTable, mLastState);
                break;
        }
        state = mLastState;
        state.playing = true;
        mInit = false;
    } else {
        state = mLastState;
    }


    if (mIr) {
        mIr->step(state);
    }

    mFc->step();
    state.frequency = mFc->frequency();

    if (mRetrigger) {
        state.retrigger = true;
        mRetrigger = false;
    }

    switch (mCh) {
        case ChType::ch1:
            ChannelControl<ChType::ch1>::update(rc.apu, rc.waveTable, mLastState, state);
            break;
        case ChType::ch2:
            ChannelControl<ChType::ch2>::update(rc.apu, rc.waveTable, mLastState, state);
            break;
        case ChType::ch3:
            ChannelControl<ChType::ch3>::update(rc.apu, rc.waveTable, mLastState, state);
            break;
        case ChType::ch4:
            ChannelControl<ChType::ch4>::update(rc.apu, rc.waveTable, mLastState, state);
            break;
    }

    state.retrigger = false;
    mLastState = state;

}

void InstrumentPreview::restart() {
    mRetrigger = true;
    if (mInstrument) {
        mIr.emplace(*mInstrument);
    }
    mFc->useInstrument(mInstrument.get());
}

}
