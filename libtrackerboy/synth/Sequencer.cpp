
#include "gbsynth.hpp"

// 512 Hz, 4194304 / 512 = 8192 cycles per step
#define FREQ_COUNTER_MAX 8192

#define allLengthStep(cf) do { \
    cf.ch1.lengthStep(); \
    cf.ch2.lengthStep(); \
    cf.ch3.lengthStep(); \
    cf.ch4.lengthStep(); \
    } while (0)

#define allEnvStep(cf) do { \
    cf.ch1.envStep(); \
    cf.ch2.envStep(); \
    cf.ch4.envStep(); \
    } while (0)

namespace gbsynth {

Sequencer::Sequencer(ChannelFile &cf) : 
    cf(cf),
    freqCounter(0),
    stepCounter(0)
{
}

void Sequencer::reset() {
    freqCounter = 0;
    stepCounter = 0;
}

void Sequencer::step(unsigned cycles) {
    // TODO: check that cycles <= FREQ_COUNTER_MAX
    freqCounter += cycles;
    if (freqCounter >= FREQ_COUNTER_MAX) {
        freqCounter -= FREQ_COUNTER_MAX;
            
        switch (stepCounter++) {
            case 0:
                allLengthStep(cf);
                break;
            case 1:
                break;
            case 2:
                allLengthStep(cf);
                cf.ch1.sweepStep();
                break;
            case 3:
                break;
            case 4:
                allLengthStep(cf);
                break;
            case 5:
                break;
            case 6:
                allLengthStep(cf);
                cf.ch1.sweepStep();
                break;
            case 7:
                stepCounter = 0; // adjust counter since it overflowed
                allEnvStep(cf);
                break;

        }
    }
        
}

}