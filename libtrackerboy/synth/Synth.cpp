
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

namespace trackerboy {


Synth::Synth(float samplingRate) :
    mCf(),
    mSequencer(mCf),
    mSamplingRate(samplingRate),
    mStepsPerSample(static_cast<uint32_t>((Gbs::CLOCK_SPEED / samplingRate) * (1 << 25))),
    mStepCounter(0)
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

    mStepCounter += mStepsPerSample;
    unsigned steps = mStepCounter >> 25; // floor
    mStepCounter &= 0x1FFFFFF; // remove integer portion
    if (mStepCounter >= 0x1000000) {
        ++steps; // round up
    }

    unsigned stepsRun;
    do {
        stepsRun = mSequencer.step(steps);
        mCf.ch1.step(stepsRun);
        mCf.ch2.step(stepsRun);
        mCf.ch3.step(stepsRun);
        mCf.ch4.step(stepsRun);
    } while (steps -= stepsRun);
    // if we stopped at a fence, this loops more than once
    
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