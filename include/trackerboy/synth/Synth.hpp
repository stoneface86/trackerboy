#pragma once

#include "ChannelFile.hpp"
#include "Mixer.hpp"
#include "Sequencer.hpp"


namespace trackerboy {

class Synth {
    ChannelFile cf;
    Mixer mixer;
    Sequencer sequencer;

    float samplingRate;
    unsigned stepsPerSample;

public:
    Synth(float samplingRate);

    ChannelFile& getChannels();
    Mixer& getMixer();
    Sequencer& getSequencer();

    void step(float &left, float &right);

    void fill(float leftBuf[], float rightBuf[], size_t nsamples);

    void fill(float buf[], size_t nsamples);
};

}