
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

    static constexpr size_t STEP_PHASES = 32;
    static constexpr size_t STEP_COUNT = 15;
    // this is the index of the center point of the step
    // i <= STEP_CENTER: left of the step
    // i > STEP_CENTER: right of the step
    static constexpr size_t STEP_CENTER = STEP_COUNT / 2;

    static const float STEP_TABLE[STEP_PHASES][STEP_COUNT];

    //
    // POD struct containing information about a delta and its
    // location in the waveform.
    //
    struct Delta {
        int16_t change;     // the change in volume
        uint8_t location;   // location in the waveform
        int16_t before;     // volume before the transition
        int16_t after;      // volume after the transition
        /* to be used by generatePeriods() */
        size_t samplesBefore;
        size_t samplesDuringBegin; // 0 - STEP_INDEX_CENTER
        size_t samplesDuringEnd;   // STEP_INDEX_CENTER - STEP_COUNT
        size_t samplesAfter;
        float phase;
    };

    // samplingRate / gameboy clock rate
    float mFactor;

    // gameboy frequency (0-2047)
    uint16_t mFrequency;

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

    size_t mPeriodCount;                    // number of generated waveform periods
    std::vector<int16_t> mPeriodBuf;        // sequence of generated waveform periods
    size_t mPeriodBufSize;                  // size in bytes of the period buffer (NOT the same as mPeriodBuf.size()!)
    size_t mPeriodPos;                      // position in the period buffer to copy from (aka phase)
    bool mRegenPeriod;                      // regenerate period sequence if true
    
    // if true, generate will output 0
    bool mMuted;

    void generatePeriods();

    //void generateStep(int16_t *stepBuf, size_t phase, int16_t init, int16_t change);
    


};


}