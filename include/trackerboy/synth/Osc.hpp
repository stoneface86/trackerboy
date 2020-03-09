
#pragma once

#include <cstdint>
#include <vector>

namespace trackerboy {

class Osc {

public:

    //
    // Disables oscillator output, generate will only silence. To re-enable,
    // call reset()
    //
    void disable();

    bool disabled();

    //
    // Returns the current frequency setting.
    //
    uint16_t frequency();

    //
    // Generate a given amount of samples and place them in the given buffer.
    //
    void generate(float samples[], size_t nsamples);

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

    void run(size_t nsamples);

    //
    // Sets the frequency of the waveform. Valid frequencies range from 0 to
    // 2047 inclusive. If the output frequency of the given frequency exceeds
    // nyquist frequency, then the closest frequency that is under will be used
    // instead in order to prevent aliasing.
    //
    void setFrequency(uint16_t frequency);

    // delete this later

    void fillPeriod();
    std::vector<float>& period();

    static constexpr unsigned PERIOD_BUFFER_SIZE_DEFAULT = 100;

    static constexpr unsigned PERIOD_BUFFER_SIZE_MAX = 1000;
    // 1/64 of the sampling rate ~ 16 milleseconds
    static constexpr unsigned PERIOD_BUFFER_SIZE_MIN = 16;

protected:

    Osc(float samplingRate, size_t multiplier, size_t waveformSize);

    //
    // POD struct containing information about a delta and its
    // location in the waveform.
    //
    struct Delta {
        uint8_t location;   // location in the waveform
        float change;       // the change in volume
    };

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

    float mInitialVolume;

    // if true the period will be regenerated
    bool mRegenPeriod;

    const size_t mWaveformSize;
    const size_t mMultiplier;

    static constexpr float VOLUME_MAX = 1.0f;
    static constexpr float VOLUME_MIN = -1.0f;
    static constexpr float VOLUME_STEP = 1.0f / 7.5f;

private:

    static constexpr size_t STEP_PHASES = 32;
    static constexpr size_t STEP_WIDTH = 16;
    static constexpr size_t STEP_CENTER = STEP_WIDTH / 2;

    static const float STEP_TABLE[STEP_PHASES][STEP_WIDTH];

    // scaling factor: samplingRate / gameboy clock rate
    float mFactor;

    // gameboy frequency (0-2047)
    uint16_t mFrequency;

    // highest allowable frequency that is <= the nyquist frequency
    uint16_t mNyquist;

    bool mDisabled;

    // maximum size of the period buffer in milleseconds
    unsigned mPeriodBufSize;

    std::vector<float> mPeriodBuf;
    size_t mPeriodOffset;


    // private methods

    //void fillPeriod();

};


}
