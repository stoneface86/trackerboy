
#include "trackerboy/synth/Sequencer.hpp"

#include <cassert>

// A step occurs every 8192 cycles (512 Hz)
//
// Step:                 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
// --------------------------+---+---+---+---+---+---+---+-------------------
// Len. counter (256 Hz) | x       x       x       x      
// Sweep        (128 Hz) |         x               x       
// envelope     ( 64 Hz) |                             x  
//
// Length counters aren't implemented so we ignore those triggers

namespace {

constexpr uint32_t CYCLES_PER_STEP = 8192;

constexpr uint32_t DEFAULT_FENCE = CYCLES_PER_STEP * 2;

}


namespace trackerboy {

Sequencer::Trigger const Sequencer::TRIGGER_SEQUENCE[] = {
    // step 0 trigger, next trigger: 2
    //{1, CYCLES_PER_STEP * 2, TriggerType::LC},
    // step 2 trigger, next trigger: 4
    //{2, CYCLES_PER_STEP * 2, TriggerType::LC_AND_SWEEP},
    // step 4 trigger, next trigger: 6
    //{3, CYCLES_PER_STEP * 2, TriggerType::LC},
    // step 6 trigger, next trigger: 7
    //{4, CYCLES_PER_STEP,     TriggerType::LC_AND_SWEEP},

    // step 2 trigger, next trigger: 1
    {1, CYCLES_PER_STEP * 4, TriggerType::sweep},

    // step 6 trigger, next trigger: 2
    {2, CYCLES_PER_STEP,     TriggerType::sweep},

    // step 7 trigger, next trigger: 0
    {0, CYCLES_PER_STEP * 3,     TriggerType::env}
};

Sequencer::Sequencer(HardwareFile &hf) noexcept :
    mHf(hf),
    mFence(DEFAULT_FENCE),
    mTriggerIndex(0)
{
}

void Sequencer::reset() noexcept {
    mFence = DEFAULT_FENCE;
    mTriggerIndex = 0;
}

void Sequencer::step(uint32_t cycles) noexcept {
    assert(cycles <= mFence);

    mFence -= cycles;

    if (mFence == 0) {
        Trigger const &trigger = TRIGGER_SEQUENCE[mTriggerIndex];
        switch (trigger.type) {
            case TriggerType::sweep:
                mHf.sweep1.trigger();
                break;
            case TriggerType::env:
                mHf.env1.trigger();
                mHf.env2.trigger();
                mHf.env4.trigger();
                break;
        }
        mFence = trigger.nextFence;
        mTriggerIndex = trigger.nextIndex;
    }

}


}
