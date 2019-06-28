
#include "gbsynth.h"

#include <algorithm>
#include <cmath>

using std::copy;
using std::copy_n;

namespace gbsynth {

    Oscillator::Oscillator(uint8_t waveform[], size_t nsamples) {
        referencePeriod.assign(waveform, waveform + nsamples);
        period = referencePeriod;
        counter = 0;
    }


    void Oscillator::setFrequency(float samplingRate, float frequency) {
        samplesPerPeriod = samplingRate / frequency;
        setPeriod();
    }

    void Oscillator::setWaveform(uint8_t waveform[], size_t nsamples) {
        referencePeriod.assign(waveform, waveform + nsamples);
        setPeriod();
    }

    void Oscillator::fill(uint8_t buf[], size_t nsamples) {
        size_t psize = period.size();
        size_t samplesToCopy = 0;
        if (counter != 0) {
            // we didn't start at a new period, finish this one first
            samplesToCopy = psize - counter;
            if (nsamples < samplesToCopy) {
                samplesToCopy = nsamples;
                counter += nsamples;
            } else {
                counter = 0;
            }

            copy_n(period.begin() + counter, samplesToCopy, buf);
            buf += samplesToCopy;
        }

        // copy full periods
        // osc->counter = 0 at this point

        samplesToCopy = nsamples - samplesToCopy;
        size_t nperiods = samplesToCopy / psize;
        size_t remainder = samplesToCopy % psize;
        for (size_t i = 0; i != nperiods; ++i) {
            copy(period.begin(), period.end(), buf);
            buf += psize;
        }

        // copy remainder
        if (remainder) {
            copy_n(period.begin(), remainder, buf);
            counter = remainder;
        }
    }

    void Oscillator::setPeriod() {
        size_t oldperiod = period.size();
        period.clear();

        // number of samples that represent a single sample from the reference period
        float samplesPerReference = samplesPerPeriod / referencePeriod.size();
        size_t spri;  // integer part of samplesPerReference
        float sprf;   // fractional part '
        float rem = 0.0f;
        {
            float tmp;
            sprf = modff(samplesPerReference, &tmp);
            spri = (size_t)tmp;
        }
        for (size_t i = 0; i != referencePeriod.size(); ++i) {
            for (size_t j = 0; j != spri; ++j) {
                period.push_back(referencePeriod[i]);
            }
            rem += sprf;
            if (rem >= 1.0f) {
                period.push_back(referencePeriod[i]);
                rem -= 1.0f;
            }
        }

        // adjust counter proportionally to the old period
        counter = (counter * period.size()) / oldperiod;
    }


}