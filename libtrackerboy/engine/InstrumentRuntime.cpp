
#include "trackerboy/engine/InstrumentRuntime.hpp"

#include "trackerboy/engine/ChannelControl.hpp"


namespace trackerboy {


InstrumentRuntime::InstrumentRuntime(RuntimeContext rc) :
    mRc(rc),
    mCh(ChType::ch1),
    mNc(),
    mFc(),
    mInstrument{ 0 },
    mAutoRetrigger(false)
{
}

void InstrumentRuntime::setChannel(ChType ch) {
    mCh = ch;

}

void InstrumentRuntime::setInstrument(Instrument &inst) {
    mInstrument = inst.data();
    mCh = static_cast<ChType>(mInstrument.channel);
    mFc.setVibrato(mInstrument.vibrato);
    mFc.setVibratoDelay(mInstrument.vibratoDelay);
    mFc.setTune(mInstrument.tune + 0x80);
    ChannelControl::writeEnvelope(mCh, mRc, mInstrument.envelope);
    ChannelControl::writeTimbre(mCh, mRc, mInstrument.timbre);
    mRc.synth.setOutputEnable(mCh, Gbs::TERM_BOTH, false);
    if (mCh == ChType::ch3) {
        mAutoRetrigger = false;
    } else {
        mAutoRetrigger = !!(mInstrument.envelope & 0x7);
    }
}

void InstrumentRuntime::playNote(Note note) {
    
    mNc.noteTrigger(note, mInstrument.delay);
    if (mInstrument.duration) {
        mNc.noteCut(mInstrument.duration);
    }
    
    
}

void InstrumentRuntime::step() {
    // NOTE: this code is similar to MusicRuntime::update, might be worth
    // extracting to a separate function usable by both.

    auto note = mNc.step();
    if (mNc.isPlaying()) {

        if (note) {

            uint8_t noteVal = note.value();

            if (mCh != ChType::ch4) {
                mFc.setNote(noteVal);
                mFc.apply();
            } else {
                // CH4
                if (noteVal <= NOTE_NOISE_LAST) {
                    uint8_t noise = NOTE_NOISE_TABLE[noteVal];
                    if (mInstrument.timbre) {
                        // nonzero timbre, 7-bit step width
                        noise |= 0x08;
                    }
                    mRc.synth.writeRegister(Gbs::REG_NR43, noise);
                }

            }

            if (mAutoRetrigger) {
                mRc.synth.restart(mCh);
            }

            // output on
            mRc.synth.setOutputEnable(mCh, Gbs::TERM_LEFT, !!(mInstrument.panning & 0x10));
            mRc.synth.setOutputEnable(mCh, Gbs::TERM_RIGHT, !!(mInstrument.panning & 0x01));
        }


        if (mCh != ChType::ch4) {
            mFc.step();
            // write frequency
            mRc.synth.setFrequency(mCh, mFc.frequency());
            
        }
    } else {
        // output off
        mRc.synth.setOutputEnable(mCh, Gbs::TERM_BOTH, false);
    }
}




}
