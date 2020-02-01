
#pragma once

#include <cstdint>
#include <vector>

namespace trackerboy {

class Osc {

public:

    //
    // Returns the current frequency setting.
    //
    uint16_t frequency();

    //
    // Generate a given amount of samples and place them in the given buffer.
    //
    void generate(int16_t samples[], size_t nsamples);

    //
    // Returns true if the oscillator is muted
    //
    bool muted();

    //
    // Returns the actual frequency being outputted, in hertz.
    //
    float outputFrequency();

    //
    // Reset the oscilator to its initial state. Sample generation will now
    // begin at the start of the waveform's period. The frequency and waveform
    // is unchanged.
    //
    void reset();

    //
    // Sets the frequency of the waveform. Valid frequencies range from 0 to
    // 2047 inclusive. If the output frequency of the given frequency exceeds
    // nyquist frequency, then the closest frequency that is under will be used
    // instead in order to prevent aliasing.
    //
    void setFrequency(uint16_t frequency);

    //
    // Mute the oscillator. When muted, the oscillator will only output 0 (silence)
    //
    void setMute(bool muted);



protected:

    Osc(float samplingRate, size_t multiplier, size_t waveformSize);

    //
    // Setup the delta buffer from the given waveform
    //
    void deltaSet(const uint8_t waveform[]);

    const size_t mWaveformSize;
    const size_t mMultiplier;

private:

    // number of phases in the sinc table
    static constexpr size_t SINC_PHASES = 32;
    // sample size per sinc set
    static constexpr size_t SINC_STEPS = 15;
    // center index of the sinc set
    static constexpr size_t SINC_CENTER = SINC_STEPS / 2;

    // table of sinc sets, each set contains samples from a normalized sinc function
    // at a given phase.
    static const float SINC_TABLE[SINC_PHASES][SINC_STEPS];

    //
    // POD struct containing information about a delta and its
    // location in the waveform.
    //
    struct Delta {
        int16_t change;     // the change in volume
        uint8_t location;   // location in the waveform
        int16_t before;     // volume before the transition
        /* to be used by generate() */
        float position;
    };

    // scaling factor: samplingRate / gameboy clock rate
    float mFactor;

    // gameboy frequency (0-2047)
    uint16_t mFrequency;

    // highest allowable frequency that is <= the nyquist frequency
    uint16_t mNyquist;

    // used by generate()

    bool mRecalc;

    float mSamplesPerDelta;

    float mSamplesPerPeriod;

    // last sample generated
    int16_t mPrevious;

    int16_t mLeftovers[SINC_STEPS - 1];

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

    // if true, generate will output 0
    bool mMuted;

    // setup timing information for generating samples at the start of the period
    void resetPeriod();

};


}
