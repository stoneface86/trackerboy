#include "gbsynth2.h"

#include <cmath>


namespace gbsynth {

    Channel::Channel(float samplingRate) {
        this->samplingRate = samplingRate;
        length = 0;
        continuous = true;
        samplesToOutput = 0;
        sampleCounter = 0;
    }


    void Channel::fill(float buf[], size_t bufsize) {
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
                // zero fill the rest of the buffer
                for (size_t i = samplesToGenerate; i != bufsize; ++i) {
                    buf[i] = 0.0f;
                }
            }
            // update the counter
            // cast is there is to silence warning
            sampleCounter += (unsigned)samplesToGenerate;
        }
        // generate the signal
        generate(buf, samplesToGenerate);

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