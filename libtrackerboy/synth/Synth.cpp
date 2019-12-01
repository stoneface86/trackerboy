
#include "trackerboy/synth/Synth.hpp"

#define GB_CLOCK_SPEED 4194304

namespace trackerboy {


Synth::Synth(float samplingRate) :
    mCf(),
    mSequencer(mCf),
    mSamplingRate(samplingRate),
    mStepsPerSample((unsigned)(GB_CLOCK_SPEED / samplingRate))
{
}

ChannelFile& Synth::getChannels() {
    return mCf;
}

Mixer& Synth::getMixer() {
    return mMixer;
}

Sequencer& Synth::getSequencer() {
    return mSequencer;
}

void Synth::step(int16_t &left, int16_t &right) {
    mMixer.getOutput(
        mCf.ch1.getCurrentVolume(),
        mCf.ch2.getCurrentVolume(),
        mCf.ch3.getCurrentVolume(),
        mCf.ch4.getCurrentVolume(),
        left,
        right
    );

    mSequencer.step(mStepsPerSample);

    mCf.ch1.step(mStepsPerSample);
    mCf.ch2.step(mStepsPerSample);
    mCf.ch3.step(mStepsPerSample);
    mCf.ch4.step(mStepsPerSample);
}


void Synth::fill(int16_t leftBuf[], int16_t rightBuf[], size_t nsamples) {
    for (size_t i = 0; i != nsamples; ++i) {
        step(leftBuf[i], rightBuf[i]);
    }
}

void Synth::fill(int16_t buf[], size_t nsamples) {
    for (size_t i = 0, j = nsamples * 2; i != j; i += 2) {
        step(buf[i], buf[i + 1]);
    }
}

}