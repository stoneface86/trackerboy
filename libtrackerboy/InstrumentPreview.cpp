
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

void InstrumentPreview::setInstrument(std::shared_ptr<Instrument> instrument) {
    mInstrument = std::move(instrument);
    if (mInstrument) {
        mCh = mInstrument->channel();
        if (mCh == ChType::ch4) {
            mFc = &mNoiseFc;
        } else {
            mFc = &mToneFc;
        }
        restart();
        mInit = true;
    } else {
        mIr.reset();
    }
}


void InstrumentPreview::play(uint8_t note) {
    if (mInstrument) {
        Operation op;
        op.note = note;
        mFc->apply(op);
        restart();
    }
}

void InstrumentPreview::step(RuntimeContext const& rc) {

    ChannelState state;
    
    if (mInit) {
        mLastState = ChannelState(mCh);
        switch (mCh) {
            case ChType::ch1:
                ChannelControl<ChType::ch1>::init(rc.apu, rc.waveList, mLastState);
                break;
            case ChType::ch2:
                ChannelControl<ChType::ch2>::init(rc.apu, rc.waveList, mLastState);
                break;
            case ChType::ch3:
                ChannelControl<ChType::ch3>::init(rc.apu, rc.waveList, mLastState);
                break;
            case ChType::ch4:
                ChannelControl<ChType::ch4>::init(rc.apu, rc.waveList, mLastState);
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
            ChannelControl<ChType::ch1>::update(rc.apu, rc.waveList, mLastState, state);
            break;
        case ChType::ch2:
            ChannelControl<ChType::ch2>::update(rc.apu, rc.waveList, mLastState, state);
            break;
        case ChType::ch3:
            ChannelControl<ChType::ch3>::update(rc.apu, rc.waveList, mLastState, state);
            break;
        case ChType::ch4:
            ChannelControl<ChType::ch4>::update(rc.apu, rc.waveList, mLastState, state);
            break;
    }

    state.retrigger = false;
    mLastState = state;

}

void InstrumentPreview::restart() {
    mRetrigger = true;
    mIr.emplace(*mInstrument);
    mToneFc.useInstrument(*mInstrument);
    mNoiseFc.useInstrument(*mInstrument);

}

}
