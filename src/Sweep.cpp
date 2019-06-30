
#include "gbsynth.h"

namespace gbsynth {

    Sweep::Sweep(PulseChannel &ch) : ch(ch) {
        // TODO
    }

    void Sweep::reset() {
        // TODO
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
        // TODO
    }


}