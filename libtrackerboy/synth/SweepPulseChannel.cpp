
#include "trackerboy/synth/SweepPulseChannel.hpp"

namespace trackerboy {

SweepPulseChannel::SweepPulseChannel() : 
    mSweepMode(Gbs::DEFAULT_SWEEP_MODE),
    mSweepTime(Gbs::DEFAULT_SWEEP_TIME),
    mSweepShift(Gbs::DEFAULT_SWEEP_SHIFT),
    mSweepCounter(0),
    PulseChannel()
{
}

void SweepPulseChannel::reset() {
    PulseChannel::reset();
    mSweepCounter = 0;
}

void SweepPulseChannel::setSweep(uint8_t sweepReg) {
    mSweepShift = sweepReg & 0x7;
    mSweepMode = static_cast<Gbs::SweepMode>((sweepReg >> 3) & 1);
    mSweepTime = (sweepReg >> 4) & 0x7;
}

void SweepPulseChannel::setSweepMode(Gbs::SweepMode mode) {
    mSweepMode = mode;
}

void SweepPulseChannel::setSweepShift(uint8_t shift) {
    if (shift > Gbs::MAX_SWEEP_SHIFT) {
        shift = Gbs::MAX_SWEEP_SHIFT;
    }
    mSweepShift = shift;
}

void SweepPulseChannel::setSweepTime(uint8_t ts) {
    if (ts > Gbs::MAX_SWEEP_TIME) {
        ts = Gbs::MAX_SWEEP_TIME;
    }
    mSweepTime = ts;
}

void SweepPulseChannel::sweepStep() {
    if (mSweepTime) {
        if (++mSweepCounter >= mSweepTime) {
            mSweepCounter = 0;
            if (mSweepShift) {
                int16_t shadow = mFrequency;
                int16_t sweepfreq = shadow >> mSweepShift;
                if (mSweepMode == Gbs::SWEEP_SUBTRACTION) {
                    sweepfreq = shadow - sweepfreq;
                    if (sweepfreq < 0) {
                        return; // no change
                    }
                } else {
                    sweepfreq = shadow + sweepfreq;
                    if (sweepfreq > Gbs::MAX_FREQUENCY) {
                        // sweep will overflow, disable the channel
                        disable();
                        return;
                    }
                }
                setFrequency(static_cast<uint16_t>(sweepfreq));
            }
        }
    }
}

}