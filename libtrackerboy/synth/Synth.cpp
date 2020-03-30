
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

#include <algorithm>
#include <climits>
#include <cmath>

namespace {

constexpr unsigned CYCLES_PER_TRIGGER = 8192;

}


namespace trackerboy {

// Frame sequencer
// this part of the APU controls when components such as sweep, envelope
// and length counters are triggered. The sequencer itself is stepped every
// 8192 cycles or at 512 Hz. The table below shows which steps the components
// are triggered (except for length counters).
//
// Step:                 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
// --------------------------+---+---+---+---+---+---+---+-------------------
// Sweep        (128 Hz) |         x               x       
// envelope     ( 64 Hz) |                             x 
//
// The synthesizer emulates this by generating samples before each trigger,
// applying the trigger, and then repeating until the buffer is filled.
//
// The time between each trigger is stored in the mTriggerTimings array. For
// the first element, this value is the number of samples needed until the
// first trigger (which is Sweep @ step 2).
//

Synth::TriggerType const Synth::TRIGGER_SEQUENCE[] = {
    TriggerType::SWEEP,     // Sweep    @ step 2
    TriggerType::SWEEP,     // Sweep    @ step 6
    TriggerType::ENV        // Envelope @ step 7
};


Synth::Synth(float samplingRate, float framerate) :
    mSamplingRate(samplingRate),
    mFramerate(framerate),
    mHf(),
    mMixer(samplingRate),
    mCyclesPerSample(Gbs::CLOCK_SPEED / samplingRate),
    mCyclesPerFrame(Gbs::CLOCK_SPEED / framerate),
    mCycleOffset(0.0f),
    mFrameBuf(static_cast<size_t>(samplingRate / framerate)),
    mTriggerTimings{
        (CYCLES_PER_TRIGGER * 3) / mCyclesPerSample, // 3 steps from 7 to 2
        (CYCLES_PER_TRIGGER * 4) / mCyclesPerSample, // 4 steps from 2 to 6
        (CYCLES_PER_TRIGGER * 1) / mCyclesPerSample  // 1 step from  6 to 7
    },
    mSampleCounter(0.0f),
    mSamplesToTrigger(mTriggerTimings[0]),
    mTriggerIndex(0),
    mOutputStat(Gbs::OUT_ALL)
{
    // NOTE: mTriggerTimings will need to be recalculated if the sampling rate
    // changes. Currently there is no way to change it after construction,
    // although this may change in the future.
}

float* Synth::buffer() {
    return mFrameBuf.data();
}

HardwareFile& Synth::hardware() {
    return mHf;
}

void Synth::fill(float buf[], size_t nsamples) {
    // TODO: implement later
}

size_t Synth::run() {
    // TODO: implement later
    return 0;

}

uint8_t Synth::readRegister(uint16_t addr) {
    // TODO
    return 0;
}


void Synth::reset() {
    // TODO: implement later
}


void Synth::restart(ChType ch) {
    switch (ch) {
        case ChType::ch1:
            mHf.env1.restart();
            mHf.sweep1.restart();
            mHf.gen1.restart();
            break;
        case ChType::ch2:
            mHf.env2.restart();
            mHf.gen2.restart();
            break;
        case ChType::ch3:
            mHf.gen3.restart();
            break;
        case ChType::ch4:
            mHf.env4.restart();
            mHf.gen4.restart();
            break;
    }
}


void Synth::setOutputEnable(Gbs::OutputFlags flags) {
    mOutputStat = flags;
}

void Synth::setOutputEnable(ChType ch, Gbs::Terminal term, bool enabled) {
    uint8_t flag = 0;
    if (term & Gbs::TERM_LEFT) {
        flag = 1 << static_cast<uint8_t>(ch);
    }

    if (term & Gbs::TERM_RIGHT) {
        flag |= 16 << static_cast<uint8_t>(ch);
    }

    if (enabled) {
        mOutputStat |= flag;
    } else {
        mOutputStat &= ~flag;
    }
}

void Synth::writeRegister(uint16_t addr, uint8_t value) {
    // TODO
}

}
