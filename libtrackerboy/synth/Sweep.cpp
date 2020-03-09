
#include "trackerboy/synth/Sweep.hpp"


namespace trackerboy {


Sweep::Sweep(Osc &osc) :
    mOsc(osc),
    mSweepMode(Gbs::DEFAULT_SWEEP_MODE),
    mSweepTime(Gbs::DEFAULT_SWEEP_TIME),
    mSweepShift(Gbs::DEFAULT_SWEEP_SHIFT),
    mSweepCounter(0)
{
}

void Sweep::reset() {
    mSweepCounter = 0;
}

void Sweep::setRegister(uint8_t reg) {
    mSweepShift = reg & 0x7;
    mSweepMode = static_cast<Gbs::SweepMode>((reg >> 3) & 1);
    mSweepTime = (reg >> 4) & 0x7;
}

void Sweep::trigger() {
    if (mSweepTime) {
        if (++mSweepCounter >= mSweepTime) {
            mSweepCounter = 0;
            if (mSweepShift) {
                int16_t shadow = mOsc.frequency();
                int16_t sweepfreq = shadow >> mSweepShift;
                if (mSweepMode == Gbs::SWEEP_SUBTRACTION) {
                    sweepfreq = shadow - sweepfreq;
                    if (sweepfreq < 0) {
                        return; // no change
                    }
                } else {
                    sweepfreq = shadow + sweepfreq;
                    if (sweepfreq > Gbs::MAX_FREQUENCY) {
                        // sweep will overflow, disable the oscillator
                        mOsc.disable();
                        return;
                    }
                }
                mOsc.setFrequency(static_cast<uint16_t>(sweepfreq));
            }
        }
    }
}



}
