
#include "trackerboy/synth/Sweep.hpp"


namespace trackerboy {


Sweep::Sweep(Osc &osc) :
    mOsc(osc),
    mSweepMode(Gbs::DEFAULT_SWEEP_MODE),
    mSweepTime(Gbs::DEFAULT_SWEEP_TIME),
    mSweepShift(Gbs::DEFAULT_SWEEP_SHIFT),
    mSweepCounter(0),
    mRegister(Gbs::DEFAULT_SWEEP_REGISTER),
    mShadow(0)
{
}

void Sweep::reset() {
    mSweepCounter = 0;
    mSweepShift = mRegister & 0x7;
    mSweepMode = static_cast<Gbs::SweepMode>((mRegister >> 3) & 1);
    mSweepTime = (mRegister >> 4) & 0x7;
    mShadow = mOsc.frequency();
}

void Sweep::setRegister(uint8_t reg) {
    mRegister = reg;
}

void Sweep::trigger() {
    if (mSweepTime) {
        if (++mSweepCounter >= mSweepTime) {
            mSweepCounter = 0;
            if (mSweepShift) {
                int16_t sweepfreq = mShadow >> mSweepShift;
                if (mSweepMode == Gbs::SWEEP_SUBTRACTION) {
                    sweepfreq = mShadow - sweepfreq;
                    if (sweepfreq < 0) {
                        return; // no change
                    }
                } else {
                    sweepfreq = mShadow + sweepfreq;
                    if (sweepfreq > Gbs::MAX_FREQUENCY) {
                        // sweep will overflow, disable the oscillator
                        mOsc.disable();
                        return;
                    }
                }
                // no overflow/underflow
                // write-back the shadow register to CH1's frequency register
                mOsc.setFrequency(static_cast<uint16_t>(sweepfreq));
                mShadow = sweepfreq;
            }
        }
    }
}



}
