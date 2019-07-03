
#include "gbsynth.h"

namespace gbsynth {

    Sweep::Sweep(PulseChannel &ch) : 
        ch(ch),
        sweepMode((SweepMode)DEFAULT_SWEEP_MODE),
        sweepTime(DEFAULT_SWEEP_TIME),
        sweepShift(DEFAULT_SWEEP_SHIFT),
        counter(0)
    {
    }

    void Sweep::reset() {
        counter = 0;
    }

    void Sweep::setSweepTime(uint8_t ts) {
        if (ts > MAX_SWEEP_TIME) {
            ts = MAX_SWEEP_TIME;
        }
        sweepTime = ts;
    }

    void Sweep::setSweepMode(SweepMode mode) {
        sweepMode = mode;
    }

    void Sweep::setSweepShift(uint8_t shift) {
        if (shift > MAX_SWEEP_SHIFT) {
            shift = MAX_SWEEP_SHIFT;
        }
        sweepShift = shift;
    }

    void Sweep::step() {
        if (sweepTime) {
            if (++counter >= sweepTime) {
                counter = 0;
                if (sweepShift) {
                    int16_t shadow = ch.getFrequency();
                    int16_t sweepfreq = shadow >> sweepShift;
                    if (sweepMode == SWEEP_SUBTRACTION) {
                        sweepfreq = shadow - sweepfreq;
                        if (sweepfreq < 0) {
                            return; // no change
                        }
                    } else {
                        sweepfreq = shadow + sweepfreq;
                        if (sweepfreq > MAX_FREQUENCY) {
                            // sweep will overflow, disable the channel
                            ch.disable();
                            return;
                        }
                    }
                    ch.setFrequency((uint16_t)sweepfreq);
                }
            }
        }
    }


}