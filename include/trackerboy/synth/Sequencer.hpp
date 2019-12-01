#pragma once

#include "ChannelFile.hpp"


namespace trackerboy {

class Sequencer {
    unsigned mFreqCounter;
    unsigned mStepCounter;
    ChannelFile &mCf;

public:
    Sequencer(ChannelFile &cf);

    void reset();
    void step(unsigned cycles);
};

}