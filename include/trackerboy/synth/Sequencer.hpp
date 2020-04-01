
#pragma once

#include "trackerboy/synth/HardwareFile.hpp"


namespace trackerboy {


class Sequencer {

public:

    Sequencer(HardwareFile &hf);

    // time in cycles til the next trigger
    inline uint32_t fence() { 
        return mFence;
    }

    void reset();

    void step(uint32_t cycles);

private:

    enum class TriggerType {
        sweep,
        env
    };

    struct Trigger {
        uint32_t nextIndex;     // next index in the sequence
        uint32_t nextFence;     // next wall to stop short
        TriggerType type;       // trigger to do
    };

    static Trigger const TRIGGER_SEQUENCE[];

    HardwareFile &mHf;
    uint32_t mFence;
    uint32_t mTriggerIndex;




};



}
