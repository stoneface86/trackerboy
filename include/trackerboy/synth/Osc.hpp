
#pragma once

#include <cstdint>
#include <vector>

namespace trackerboy {

class Osc {

public:

    
    //
    // Reset the oscilator to its initial state. Sample generation will now
    // begin at the start of the waveform's period. The frequency and waveform
    // is unchanged.
    //
    void reset();

    //
    // Sets the frequency of the waveform. Valid frequencies range from 0 to
    // 2047 inclusive.
    //
    void setFrequency(uint16_t frequency);

    //
    // Mute the oscillator. When muted, the oscillator will only output 0 (silence)
    //
    void setMute(bool muted);

    //
    // Generate a given amount of samples and place them in the given buffer.
    //
    void generate(int16_t samples[], size_t nsamples);
 
protected:

    Osc(float samplingRate, size_t multiplier, size_t waveformSize);

    //
    // Setup the delta buffer from the given waveform
    //
    void deltaSet(const uint8_t waveform[]);

    const size_t mWaveformSize;
    const size_t mMultiplier;

private:

    //
    // POD struct containing information about a delta and its
    // location in the waveform.
    //
    struct Delta {
        int16_t change; // the change in volume
        uint8_t location;
        uint8_t duration;
        //float time;     // time, in samples, when this delta occurs
        //float duration; // time, in samples until the next delta
    };

    // samplingRate / gameboy clock rate
    float mFactor;

    // number of samples per waveform transition (delta)
    float mSamplesPerDelta;

    // number of samples per waveform period
    // equal to mSamplesPerDelta * mWaveformSize
    float mSamplesPerPeriod;

    // time counter, in samples, determines location in the period of the waveform
    float mSampleCounter;

    int16_t mPrevious; // last sample generated

    // the waveform is represented by amplitude changes (deltas)
    //
    // some facts:
    //  * if the sum of all changes is 0, the waveform is periodic, aperiodic otherwise
    //  * any delta buf with 1 delta is aperiodic
    //  * an empty delta buf represents a flat wave
    //  * the sum of the durations is equal to mSamplesPerPeriod
    //
    // it is considered an error for a waveform to be aperiodic (although the resulting
    // output would be periodic due to integer overflow)
    std::vector<Delta> mDeltaBuf;

    Delta mDelta;           // the current delta to generate a transition
    size_t mDeltaIndex;     // index of the current delta
    size_t mLastStepSize;   // size in samples, of the previously generated right side

    std::vector<int16_t> mTransitionBuf; // generated samples from the current delta
    size_t mTransitionBufRemaining;      // number of samples to copy out

    bool mMuted;

    void nextDelta();

};


}