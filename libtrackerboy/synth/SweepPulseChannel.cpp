
#include "trackerboy/synth.hpp"

namespace trackerboy {

SweepPulseChannel::SweepPulseChannel() : 
    sweepMode(Gbs::DEFAULT_SWEEP_MODE),
    sweepTime(Gbs::DEFAULT_SWEEP_TIME),
    sweepShift(Gbs::DEFAULT_SWEEP_SHIFT),
    sweepCounter(0),
    PulseChannel()
{
}

void SweepPulseChannel::reset() {
    PulseChannel::reset();
    sweepCounter = 0;
}

void SweepPulseChannel::setSweep(uint8_t sweepReg) {
    sweepShift = sweepReg & 0x7;
    sweepMode = static_cast<Gbs::SweepMode>((sweepReg >> 3) & 1);
    sweepTime = (sweepReg >> 4) & 0x7;
}

void SweepPulseChannel::setSweepMode(Gbs::SweepMode mode) {
    sweepMode = mode;
}

void SweepPulseChannel::setSweepShift(uint8_t shift) {
    if (shift > Gbs::MAX_SWEEP_SHIFT) {
        shift = Gbs::MAX_SWEEP_SHIFT;
    }
    sweepShift = shift;
}

void SweepPulseChannel::setSweepTime(uint8_t ts) {
    if (ts > Gbs::MAX_SWEEP_TIME) {
        ts = Gbs::MAX_SWEEP_TIME;
    }
    sweepTime = ts;
}

void SweepPulseChannel::sweepStep() {
    if (sweepTime) {
        if (++sweepCounter >= sweepTime) {
            sweepCounter = 0;
            if (sweepShift) {
                int16_t shadow = frequency;
                int16_t sweepfreq = shadow >> sweepShift;
                if (sweepMode == Gbs::SWEEP_SUBTRACTION) {
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