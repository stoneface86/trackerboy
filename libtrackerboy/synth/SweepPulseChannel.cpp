
#include "trackerboy/synth.hpp"

namespace trackerboy {

SweepPulseChannel::SweepPulseChannel() : 
    sweepMode(static_cast<SweepMode>(DEFAULT_SWEEP_MODE)),
    sweepTime(DEFAULT_SWEEP_TIME),
    sweepShift(DEFAULT_SWEEP_SHIFT),
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
    sweepMode = static_cast<SweepMode>((sweepReg >> 3) & 1);
    sweepTime = (sweepReg >> 4) & 0x7;
}

void SweepPulseChannel::setSweepMode(SweepMode mode) {
    sweepMode = mode;
}

void SweepPulseChannel::setSweepShift(uint8_t shift) {
    if (shift > MAX_SWEEP_SHIFT) {
        shift = MAX_SWEEP_SHIFT;
    }
    sweepShift = shift;
}

void SweepPulseChannel::setSweepTime(uint8_t ts) {
    if (ts > MAX_SWEEP_TIME) {
        ts = MAX_SWEEP_TIME;
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
                if (sweepMode == SweepMode::subtraction) {
                    sweepfreq = shadow - sweepfreq;
                    if (sweepfreq < 0) {
                        return; // no change
                    }
                } else {
                    sweepfreq = shadow + sweepfreq;
                    if (sweepfreq > MAX_FREQUENCY) {
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