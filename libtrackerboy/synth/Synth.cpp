
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

#include <algorithm>
#include <climits>

namespace {

constexpr unsigned CYCLES_PER_TRIGGER = 8192;

static const float GAIN_TABLE[16] = {
    0.0f,               // Envelope = 0, no gain
    1.0f / 64.0f,
    2.0f / 64.0f,
    3.0f / 64.0f,
    4.0f / 64.0f,
    5.0f / 64.0f,
    6.0f / 64.0f,
    7.0f / 64.0f,
    8.0f / 64.0f,
    9.0f / 64.0f,
    10.0f / 64.0f,
    11.0f / 64.0f,
    12.0f / 64.0f,
    13.0f / 64.0f,
    14.0f / 64.0f,
    15.0f / 64.0f       // Envelope = 0xF, max gain approx 1/4
};


enum class Panning {
    none = 0,
    left = 1,
    right = 2,
    both = 3
};

template <Panning panning>
void mix(float *inptr, float *outptr, float gain, size_t nsamples) {

    for (size_t i = 0; i != nsamples; ++i) {
        float sample = gain * *inptr++;
        switch (panning) {
            case Panning::none:
                return;
            case Panning::left:
                *outptr++ += sample;
                outptr++;
                break;
            case Panning::right:
                outptr++;
                *outptr++ += sample;
                break;
            case Panning::both:
                *outptr++ += sample;
                *outptr++ += sample;
                break;
        }
    }
}


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


Synth::Synth(float samplingRate) :
    mSamplingRate(samplingRate),
    mHf(samplingRate),
    mCyclesPerSample(Gbs::CLOCK_SPEED / samplingRate),
    mTriggerTimings{
        (CYCLES_PER_TRIGGER * 3) / mCyclesPerSample, // 3 steps from 7 to 2
        (CYCLES_PER_TRIGGER * 4) / mCyclesPerSample, // 4 steps from 2 to 6
        (CYCLES_PER_TRIGGER * 1) / mCyclesPerSample  // 1 step from  6 to 7
    },
    mSampleCounter(0.0f),
    mSamplesToTrigger(mTriggerTimings[0]),
    mTriggerIndex(0),
    mInputBuffer(1 + static_cast<size_t>((CYCLES_PER_TRIGGER * 4) / mCyclesPerSample)),
    mOutputStat(Gbs::OUT_ALL)
{
    // NOTE: mTriggerTimings will need to be recalculated if the sampling rate
    // changes. Currently there is no way to change it after construction,
    // although this may change in the future.
}

HardwareFile& Synth::hardware() {
    return mHf;
}

void Synth::fill(float buf[], size_t nsamples) {
    
    float *inbuf = mInputBuffer.data();
    // clear output buffer
    std::fill_n(buf, nsamples * 2, 0.0f);
    
    while (nsamples != 0) {

        float samples = mSamplesToTrigger - mSampleCounter;
        size_t samplesWhole = static_cast<size_t>(samples);

        TriggerType trigger;

        if (samplesWhole <= nsamples) {
            // enough samples to hit the trigger, set it
            trigger = TRIGGER_SEQUENCE[mTriggerIndex];
            // update index to the next one, rollback to start if needed
            if (++mTriggerIndex == TRIGGER_COUNT) {
                mTriggerIndex = 0;
            }

            // figure out the timing for the next trigger
            // add the fractional part since we can only generate whole samples
            mSamplesToTrigger = mTriggerTimings[mTriggerIndex] + (samples - samplesWhole);
            mSampleCounter = 0.0f;

        } else {
            // not enough samples in buffer to hit the trigger
            // this also means we are at the end of the buffer
            samplesWhole = nsamples;
            // adjust the counter for the next time
            mSampleCounter += samplesWhole;
            // don't trigger anything
            trigger = TriggerType::NONE;
        }

        // generate the samples and sum into buf for each oscillator/generator
        
        run<ChType::ch1>(inbuf, buf, samplesWhole);
        run<ChType::ch2>(inbuf, buf, samplesWhole);
        run<ChType::ch3>(inbuf, buf, samplesWhole);
        run<ChType::ch4>(inbuf, buf, samplesWhole);

        // do trigger
        switch (trigger) {
            case TriggerType::NONE:
                break;
            case TriggerType::ENV:
                mHf.env1.trigger();
                mHf.env2.trigger();
                mHf.env4.trigger();
                break;
            case TriggerType::SWEEP:
                mHf.sweep1.trigger();
                break;
        }
        
        nsamples -= samplesWhole;
        // interleaved output buffer, 2 channels per sample
        buf += (samplesWhole * 2);
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

template <ChType ch>
void Synth::run(float inbuf[], float outbuf[], size_t nsamples) {

    // first, determine if we will actually output sound. If not, we can skip
    // generating samples and just "run" the oscillators/generators.
    //
    // Reasons for not outputting sound:
    // 1. envelope is 0 (channels 1, 2, and 4)
    // 2. oscillator is disabled (channels 1, 2, and 3)
    // 3. oscillator frequency is too high (above nyquist)
    // 4. both terminals are off for this channel

    // get the panning for this channel
    constexpr auto chInt = static_cast<uint8_t>(ch);
    uint8_t chstat = ((0x11 << chInt) & mOutputStat);
    

    bool hasOutput = false;
    if (chstat) {
        // if chstat == 0 then both terminals for the channel are disabled
        // now we must check the hardware
        switch (ch) {
            case ChType::ch1:
                hasOutput = mHf.env1.value() != 0 && !mHf.osc1.disabled();
                break;
            case ChType::ch2:
                hasOutput = mHf.env2.value() != 0 && !mHf.osc2.disabled();
                break;
            case ChType::ch3:
                hasOutput = !mHf.osc3.disabled();
                break;
            case ChType::ch4:
                hasOutput = mHf.env4.value() != 0;
                break;
        }
    }

    if (hasOutput) {
        // we have output, generate samples into inbuf

        Envelope *env = nullptr;

        switch (ch) {
            case ChType::ch1:
                mHf.osc1.generate(inbuf, nsamples);
                env = &mHf.env1;
                break;
            case ChType::ch2:
                mHf.osc2.generate(inbuf, nsamples);
                env = &mHf.env2;
                break;
            case ChType::ch3:
                mHf.osc3.generate(inbuf, nsamples);
                break;
            case ChType::ch4:
                mHf.gen4.generate(inbuf, nsamples, mCyclesPerSample);
                env = &mHf.env4;
                break;

        }
        
        Panning panning = static_cast<Panning>(
            ((chstat & 0xF) >> chInt) | (chstat >> (3 + chInt))
                );

        // mix the generated samples into the output buffer

        float gain;
        if (env == nullptr) {
            gain = GAIN_TABLE[15];
        } else {
            gain = GAIN_TABLE[env->value()];
        }

        float *inptr = inbuf;
        float *outptr = outbuf;


        switch (panning) {
            case Panning::none:
                break; // should never happen
            case Panning::left:
                mix<Panning::left>(inptr, outptr, gain, nsamples);
                break;
            case Panning::right:
                mix<Panning::right>(inptr, outptr, gain, nsamples);
                break;
            case Panning::both:
                mix<Panning::both>(inptr, outptr, gain, nsamples);
                break;
        }

    } else {
        // no output, just run the hardware (running the hardware is the same
        // as generate except without actually generating samples).
        switch (ch) {
            case ChType::ch1:
                mHf.osc1.run(nsamples);
                break;
            case ChType::ch2:
                mHf.osc2.run(nsamples);
                break;
            case ChType::ch3:
                mHf.osc3.run(nsamples);
                break;
            case ChType::ch4:
                mHf.gen4.run(nsamples, mCyclesPerSample);
                break;

        }


    }
    
}


}