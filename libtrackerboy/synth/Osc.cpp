
#include "trackerboy/synth/Osc.hpp"
#include "trackerboy/gbs.hpp"
#include "./sampletable.hpp"

#include <algorithm>


// Notes:
//
// The oscillator fills a sample buffer from a set waveform and frequency
// The waveform is composed of "segments", which take up 4 bits (0-F). A
// segment is converted to a line of samples. The length of this line
// depends on the frequency and sampling rate. When the length is less than
// 1, linear interpolation is used between the sample of the current segment
// and the previous sample.
//
// Template parameters: the multiplier is used for frequency calculation,
// the pulse channel uses 4 and the wave channel uses 2 (see the hardware
// documentation for more details on this). The segments parameter determines
// the number of segments in the waveform. Pulse has 8, wave has 32.
//
// TODO: add bandlimited steps for transitions between segments instead of
//       linear interpolation
// TODO: replace PulseChannel and WaveChannel step methods with this
//


namespace trackerboy {

template <unsigned multiplier, size_t segments>
Osc<multiplier, segments>::Osc(float samplingRate) :
    mFactor(samplingRate / Gbs::CLOCK_SPEED),
    mSamplesPerSegment(2048 * multiplier * mFactor),
    mSampleCounter(mSamplesPerSegment),
    mCurrentSegment(0),
    mCurrentDoubleSegment(0),
    mWaveform{ 0 },
    mWaveIndex(0),
    mPrevious(0)
{
}

template <unsigned multiplier, size_t segments>
void Osc<multiplier, segments>::reset() {
    mSampleCounter = mSamplesPerSegment;
    mCurrentDoubleSegment = mWaveform[0];
    mCurrentSegment = mCurrentDoubleSegment >> 4;
    mWaveIndex = 0;
    mPrevious = 0;
}

template <unsigned multiplier, size_t segments>
void Osc<multiplier, segments>::setFrequency(uint16_t frequency) {
    mSamplesPerSegment = (2048 - frequency) * multiplier * mFactor;
    mSampleCounter = mSamplesPerSegment;
}

template <unsigned multiplier, size_t segments>
void Osc<multiplier, segments>::setWaveform(uint8_t waveform[segments / 2]) {
    std::copy_n(waveform, segments / 2, mWaveform);
    mCurrentDoubleSegment = mWaveform[mWaveIndex >> 1];
    if (mWaveIndex & 1) {
        // odd, low nibble
        mCurrentSegment = mCurrentDoubleSegment & 0xF;
    } else {
        // even, high nibble
        mCurrentSegment = mCurrentDoubleSegment >> 4;
    }
}

template <unsigned multiplier, size_t segments>
void Osc<multiplier, segments>::generate(int16_t samples[], size_t nsamples) {
    int16_t currentSample = 0;
    for (size_t i = 0; i != nsamples; ++i) {
        if (mSampleCounter > 1.0f) {
            currentSample = SAMPLE_TABLE[mCurrentSegment + 240];
            samples[i] = currentSample;
            mSampleCounter -= 1.0f;
        } else if (mSampleCounter > 0.0f) {
            // get the next segment of the waveform
            if (++mWaveIndex >= segments) {
                mWaveIndex = 0;
            }
            if (mWaveIndex & 1) {
                // odd number, low nibble
                mCurrentSegment = mCurrentDoubleSegment & 0xF;
            } else {
                // even number, high nibble and load from waveform
                mCurrentDoubleSegment = mWaveform[mWaveIndex >> 1];
                mCurrentSegment = mCurrentDoubleSegment >> 4;
            }
            
            if (mSampleCounter > 0.0f) {
                // linear interpolation
                currentSample = mPrevious + ((SAMPLE_TABLE[mCurrentSegment + 240] - mPrevious) * mSampleCounter);
                samples[i] = currentSample;
                mSampleCounter += mSamplesPerSegment;
                if (mSampleCounter > 1.0f) {
                    // the interpolated sample counts towards the next segment
                    mSampleCounter -= 1.0f;
                }
            } else {
                --i; // we didn't output a sample here
            }

            
            
        }

        mPrevious = currentSample;
    }
}




template class Osc<4, 8>;
template class Osc<2, 32>;

}