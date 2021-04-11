
#include "trackerboy/engine/TrackRuntime.hpp"
#include "trackerboy/note.hpp"

#include "internal/enumutils.hpp"

#include "gbapu.hpp"


namespace trackerboy {



TrackRuntimeBase::TrackRuntimeBase(ChType ch, FrequencyControl &fc) :
    mCh(ch),
    mFc(fc),
    mRegAddr(gbapu::Apu::REG_NR10 + (+ch * GB_CHANNEL_REGS))
{
}

void TrackRuntimeBase::setRow(RuntimeContext const& rc, TrackRow const& row) {
    if (row.isEmpty()) {
        // empty row, do nothing
        return;
    }

    mNewRow = true;
    mCurrentRow = row;
}

void TrackRuntimeBase::step(RuntimeContext const &rc) {
    //if (mNewRow) {
    //    // update instrument
    //    auto instrumentId = mCurrentRow.queryInstrument();
    //    if (instrumentId) {
    //        auto inst = rc.instList[*instrumentId];
    //        if (inst != nullptr) {
    //            mInstrument = inst;
    //        }
    //    }


    //    // figure out what delay to use, here is the order we check
    //    // 1. effect setting if exists (Gxx)
    //    // 2. instrument delay if not 0
    //    // 3. 0

    //    std::optional<uint8_t> delay;

    //    for (size_t i = 0; i != TrackRow::MAX_EFFECTS; ++i) {
    //        auto effect = mCurrentRow.queryEffect(i);
    //        if (effect && effect->type == EffectType::delayedNote) {
    //            // #1
    //            delay = effect->param;
    //            break;
    //        }
    //    }

    //    if (!delay) {
    //        if (mInstrument != nullptr) {
    //            // #2
    //            delay = mInstrument->delay();
    //        } else {
    //            // #3
    //            delay = 0;
    //        }
    //    }

    //    if (*delay) {
    //        mDelayCounter = *delay;
    //        mDelayingRow = true;
    //    } else {
    //        // no delay, handle this immediately
    //        handleRow(rc);
    //        mDelayingRow = false;
    //    }
    //    mNewRow = false;
    //}

    //if (mDelayingRow) {
    //    if (mDelayCounter == 0) {
    //        handleRow(rc);
    //        mDelayingRow = false;
    //    } else {
    //        mDelayCounter--;
    //    }
    //}

    //


    //switch (mState) {
    //    case State::delayed:
    //        if (mDelayCounter == 0) {
    //            // process row
    //            FrequencyControl::Parameters params;

    //            mState = State::playing;
    //        } else {
    //            --mDelayCounter;
    //            break;
    //        }
    //        [[fallthrough]];
    //    case State::playing:
    //}

}

void TrackRuntimeBase::handleRow(RuntimeContext const& rc) {
    // process the row
    //FrequencyControl::Parameters params;

    //// effects
    //for (size_t i = 0; i != TrackRow::MAX_EFFECTS; ++i) {
    //    auto effect = mCurrentRow.queryEffect(i);
    //    if (effect) {
    //        if ((+effect->type & 0xC0) == EFFECT_CATEGORY_FREQ) {
    //            params.setEffect(effect->type, effect->param);
    //        } else {
    //            switch (effect->type) {

    //            }
    //        }
    //    }
    //}


    //bool instrumentReset = false;
    //auto note = mCurrentRow.queryNote();
    //if (note) {
    //    if (*note == NOTE_CUT) {

    //    } else {
    //        params.setNote(*note);
    //        instrumentReset = true;
    //    }
    //}

    //if (mCurrentRow.queryInstrument()) {
    //    instrumentReset = true;
    //}

    //if (instrumentReset && mInstrument) {

    //    mPanningSequence = mInstrument->sequence(Instrument::SEQUENCE_PANNING).enumerator();
    //    mTimbreSequence = mInstrument->sequence(Instrument::SEQUENCE_TIMBRE).enumerator();
    //    params.setArpSequence(mInstrument->sequence(Instrument::SEQUENCE_ARP));
    //    params.setPitchSequence(mInstrument->sequence(Instrument::SEQUENCE_PITCH));
    //}

}


// ---------------------------------------------------


template <ChType ch>
TrackRuntime<ch>::TrackRuntime() :
    TrackRuntimeBase(ch, mFcImpl),
    mFcImpl()
{
}

template class TrackRuntime<ChType::ch1>;
template class TrackRuntime<ChType::ch2>;
template class TrackRuntime<ChType::ch3>;
template class TrackRuntime<ChType::ch4>;

}
