
#include "trackerboy/synth/utils.hpp"
#include "trackerboy/gbs.hpp"

#include <cmath>


namespace trackerboy {

float fromGbFreq(uint16_t freq) {
    if (freq > Gbs::MAX_FREQUENCY) {
        freq = Gbs::MAX_FREQUENCY; // clamp
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


    return Gbs::MAX_FREQUENCY;
}

}