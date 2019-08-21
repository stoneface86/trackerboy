
#include "gbsynth.hpp"

#define GB_CLOCK_SPEED 4194304

namespace gbsynth {


Synth::Synth(float samplingRate) :
    cf(),
    sequencer(cf),
    samplingRate(samplingRate),
    stepsPerSample((unsigned)(GB_CLOCK_SPEED / samplingRate))
{
}

ChannelFile& Synth::getChannels() {
    return cf;
}

Mixer& Synth::getMixer() {
    return mixer;
}

Sequencer& Synth::getSequencer() {
    return sequencer;
}

void Synth::step(float &left, float &right) {
    mixer.getOutput(
        cf.ch1.getCurrentVolume(),
        cf.ch2.getCurrentVolume(),
        cf.ch3.getCurrentVolume(),
        cf.ch4.getCurrentVolume(),
        left,
        right
    );

    sequencer.step(stepsPerSample);

    cf.ch1.step(stepsPerSample);
    cf.ch2.step(stepsPerSample);
    cf.ch3.step(stepsPerSample);
    cf.ch4.step(stepsPerSample);
}


void Synth::fill(float leftBuf[], float rightBuf[], size_t nsamples) {
    for (size_t i = 0; i != nsamples; ++i) {
        step(leftBuf[i], rightBuf[i]);
    }
}

void Synth::fill(float buf[], size_t nsamples) {
    for (size_t i = 0; i != nsamples; i += 2) {
        step(buf[i], buf[i + 1]);
    }
}

}