
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
    uint8_t panning = mRc.apu.readRegister(gbapu::Apu::REG_NR51);
    panning &= ~(0x11 << static_cast<int>(mCh));
    mRc.apu.writeRegister(gbapu::Apu::REG_NR51, panning);
    //mRc.synth.setOutputEnable(mCh, Gbs::TERM_BOTH, false);
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

        uint16_t retrigger = 0;

        if (note) {

            uint8_t noteVal = note.value();

            if (mAutoRetrigger) {
                retrigger = 0x8000;
            }

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
                    ChannelControl::writeFrequency(ChType::ch4, mRc, retrigger | noise);
                }

            }



            // output on
            uint8_t panning = mRc.apu.readRegister(gbapu::Apu::REG_NR51);
            panning |= mInstrument.panning << static_cast<int>(mCh);
            mRc.apu.writeRegister(gbapu::Apu::REG_NR51, panning);
        }


        if (mCh != ChType::ch4) {
            mFc.step();
            // write frequency
            ChannelControl::writeFrequency(mCh, mRc, retrigger | mFc.frequency());
            
        }
    } else {
        // output off
        uint8_t panning = mRc.apu.readRegister(gbapu::Apu::REG_NR51);
        panning &= ~(0x11 << static_cast<int>(mCh));
        mRc.apu.writeRegister(gbapu::Apu::REG_NR51, panning);
    }
}




}
