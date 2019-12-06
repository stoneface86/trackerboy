#pragma once

#include "ChannelFile.hpp"


namespace trackerboy {

class Sequencer {
    

public:
    Sequencer(ChannelFile &cf);

    void reset();
    unsigned step(unsigned cycles);


private:

    enum TriggerType {
        NONE,
        LC,
        LC_AND_SWEEP,
        ENV
    };
    struct Trigger {
        unsigned nextIndex;     // next index in the sequence
        unsigned nextFence;      // next wall to stop short
        TriggerType trigger;    // trigger to do
    };

    static Trigger const TRIGGER_SEQUENCE[];
    
    ChannelFile &mCf;
    unsigned mFreqCounter;
    unsigned mFence;
    unsigned mTriggerIndex;
    TriggerType mTrigger;

    

};

}