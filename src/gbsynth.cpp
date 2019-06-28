
#include "gbsynth.h"

#include <cmath>


namespace gbsynth {

    static const float SAMPLE_TABLE[] = {
        -1.0f,
        -7.0f / 8.0f,
        -6.0f / 8.0f,
        -5.0f / 8.0f,
        -4.0f / 8.0f,
        -3.0f / 8.0f,
        -2.0f / 8.0f,
        -1.0f / 8.0f,
         0.0f,
         1.0f / 7.0f,
         2.0f / 7.0f,
         3.0f / 7.0f,
         4.0f / 7.0f,
         5.0f / 7.0f,
         6.0f / 7.0f,
         1.0f
    };


    float fromGbFreq(uint16_t freq) {
        if (freq > MAX_FREQUENCY) {
            freq = MAX_FREQUENCY; // clamp
        }
        return 131072.0f / (2048 - freq);
    }

    uint16_t toGbFreq(float freq) {

        if (freq < 0) {
            return 0;
        }

        float calc = 2048.0f - (131072.0f / freq);
        if (isnormal(calc) && calc > 0.0f) {
            calc = roundf(calc);
            if (calc < 2048.0f) {
                return (uint16_t)calc;
            }
        }


        return MAX_FREQUENCY;
    }

    void bufToFloat(uint8_t input[], float output[], size_t nsamples) {
        for (size_t i = 0; i != nsamples; ++i) {
            output[i] = SAMPLE_TABLE[input[i]];
        }
    }

    void bufToInt16(uint8_t input[], int16_t output[], size_t nsamples) {
        for (size_t i = 0; i != nsamples; ++i) {

        }
    }


}