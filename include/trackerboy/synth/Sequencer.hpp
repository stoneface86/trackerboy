#pragma once

#include "ChannelFile.hpp"


namespace trackerboy {

class Sequencer {
    unsigned freqCounter;
    unsigned stepCounter;
    ChannelFile &cf;

public:
    Sequencer(ChannelFile &cf);

    void reset();
    void step(unsigned cycles);
};

}