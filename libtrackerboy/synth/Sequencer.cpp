
#include "trackerboy/synth/Sequencer.hpp"

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

namespace trackerboy {

Sequencer::Sequencer(ChannelFile &cf) : 
    mCf(cf),
    mFreqCounter(0),
    mStepCounter(0)
{
}

void Sequencer::reset() {
    mFreqCounter = 0;
    mStepCounter = 0;
}

void Sequencer::step(unsigned cycles) {
    // TODO: check that cycles <= FREQ_COUNTER_MAX
    mFreqCounter += cycles;
    if (mFreqCounter >= FREQ_COUNTER_MAX) {
        mFreqCounter -= FREQ_COUNTER_MAX;
            
        switch (mStepCounter++) {
            case 0:
                allLengthStep(mCf);
                break;
            case 1:
                break;
            case 2:
                allLengthStep(mCf);
                mCf.ch1.sweepStep();
                break;
            case 3:
                break;
            case 4:
                allLengthStep(mCf);
                break;
            case 5:
                break;
            case 6:
                allLengthStep(mCf);
                mCf.ch1.sweepStep();
                break;
            case 7:
                mStepCounter = 0; // adjust counter since it overflowed
                allEnvStep(mCf);
                break;

        }
    }
        
}

}