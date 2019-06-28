#include "gbsynth.h"

#include <cmath>


namespace gbsynth {

    Channel::Channel(float samplingRate) {
        this->samplingRate = samplingRate;
        length = 0;
        continuous = true;
        samplesToOutput = 0;
        sampleCounter = 0;
        enabled = true;
    }


    void Channel::fill(uint8_t buf[], size_t bufsize) {

        size_t ngenerated = 0;

        if (enabled) {
            size_t samplesToGenerate;
            if (continuous) {
                // continuous output, fill the entire buffer
                samplesToGenerate = bufsize;
            } else {
                // not continuous, output until sampleCounter == samplesToOutput
                // 0.0f will be outputted for rest of the buffer (if needed)
                samplesToGenerate = samplesToOutput - sampleCounter;
                if (samplesToGenerate > bufsize) {
                    samplesToGenerate = bufsize;
                } else {
                    enabled = false; // stop the sound, length reached
                }
                // update the counter
                // cast is there is to silence warning
                sampleCounter += (unsigned)samplesToGenerate;
            }
            // generate the signal
            ngenerated = generate(buf, samplesToGenerate);
        }

        // zero fill the rest of the buffer WHEN:
        // * sound was not continuous and the length was reached
        // * the channel stopped generation early (ie square sweep overflow)
        // * sound is disabled
        for (size_t i = ngenerated; i != bufsize; ++i) {
            buf[i] = SAMPLE_GND;
        }

        /*float* bp = buf;
        float sample;
        for (size_t i = 0; i != bufsize; ++i) {
            if (continuous || sampleCounter++ < samplesToOutput) {
                sample = envelope * currentSample;
                tick();
            } else {
                sample = 0.0f;
            }
            *bp++ = sample;
        }*/
    }

    void Channel::setLength(uint8_t length) {
        if (length > MAX_LENGTH) {
            length = MAX_LENGTH;
        }
        this->length = length;
        samplesToOutput = (unsigned)roundf(samplingRate * ((64 - length) / 256.0f));
    }

    void Channel::setContinuousOutput(bool continuous) {
        this->continuous = continuous;
    }
}