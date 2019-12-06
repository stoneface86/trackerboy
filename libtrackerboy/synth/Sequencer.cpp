
#include "trackerboy/synth/Sequencer.hpp"

// 512 Hz, 4194304 / 512 = 8192 cycles per step
static constexpr unsigned CYCLES_PER_STEP = 8192;


namespace trackerboy {


// A step occurs every 8192 cycles (512 Hz)
//
// Step:                 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
// --------------------------+---+---+---+---+---+---+---+-------------------
// Len. counter (256 Hz) | x       x       x       x      
// Sweep        (128 Hz) |         x               x       
// envelope     ( 64 Hz) |                             x  
//
// a fence is needed at each step where any of these components are triggered
// so that the changes caused by the sequencer propagates to the channels.
// Which in this case is steps 0, 2, 4, 6 and 7
//
// stopping at the fence allows the channels to "catch up" to the sequencer
// before the channels' LC, sweep or envelope is triggered.

// for example lets say freqCounter is 8162 and we're gonna step 60 cycles,
// and lets say doing so would trigger the length counter for all channels.
// Without a fence, when stepping 60 cycles each channel would see
// the changes to the LC for the entire 60 cycles and not just the 30 cycles after
// the trigger.
//
//     Fence
//  +----+  +-----+
//  | LC |  | LC' |
//  +----+  +-----+
//  step()  step()
//
//   No fence
//  +--------------+
//  | LC'          |
//  +--------------+
//  step()
//

Sequencer::Trigger const Sequencer::TRIGGER_SEQUENCE[] = {
    // step 0 trigger, next trigger: 2
    {1, CYCLES_PER_STEP * 2, TriggerType::LC},
    // step 2 trigger, next trigger: 4
    {2, CYCLES_PER_STEP * 2, TriggerType::LC_AND_SWEEP},
    // step 4 trigger, next trigger: 6
    {3, CYCLES_PER_STEP * 2, TriggerType::LC},
    // step 6 trigger, next trigger: 7
    {4, CYCLES_PER_STEP,     TriggerType::LC_AND_SWEEP},
    // step 7 trigger, next trigger: 0
    {0, CYCLES_PER_STEP,     TriggerType::ENV}
};

Sequencer::Sequencer(ChannelFile &cf) : 
    mCf(cf),
    mFreqCounter(0),
    mFence(CYCLES_PER_STEP * 2),
    mTriggerIndex(0),
    mTrigger(TriggerType::LC)
{
}

void Sequencer::reset() {
    mFreqCounter = 0;
    // on the first step we trigger LC
    Trigger const &t = TRIGGER_SEQUENCE[4];
    mFence = t.nextFence;
    mTriggerIndex = t.nextIndex;
    mTrigger = t.trigger;
}

unsigned Sequencer::step(unsigned cycles) {
    
    switch (mTrigger) {
        case TriggerType::NONE:
            break;
        case TriggerType::LC_AND_SWEEP:
            mCf.ch1.sweepStep();
            // fall-through
        case TriggerType::LC:
            mCf.ch1.lengthStep();
            mCf.ch2.lengthStep();
            mCf.ch3.lengthStep();
            mCf.ch4.lengthStep();
            mTrigger = TriggerType::NONE;
            break;
        case TriggerType::ENV:
            mCf.ch1.envStep();
            mCf.ch2.envStep();
            mCf.ch4.envStep();
            mTrigger = TriggerType::NONE;
            break;
    }

    unsigned stepsAvailable = mFence - mFreqCounter;
    if (cycles >= stepsAvailable) {
        // stop here, so that changes propagate through the channels properly
        mFreqCounter = 0;
        Trigger const &trigger = TRIGGER_SEQUENCE[mTriggerIndex];
        mFence = trigger.nextFence;
        mTriggerIndex = trigger.nextIndex;
        mTrigger = trigger.trigger;
        return stepsAvailable;
    } else {
        // no fence was hit, so return what we were given
        mFreqCounter += cycles;
        return cycles;
    }

    
        
}

}