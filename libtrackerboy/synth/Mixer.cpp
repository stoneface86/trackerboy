
#include "trackerboy/synth/Mixer.hpp"


namespace trackerboy {

Mixer::Mixer() :
    s01enable(Gbs::DEFAULT_TERM_ENABLE),
    s02enable(Gbs::DEFAULT_TERM_ENABLE),
    s01vol(Gbs::DEFAULT_TERM_VOLUME),
    s02vol(Gbs::DEFAULT_TERM_VOLUME),
    outputStat(Gbs::OUT_OFF)
{
}

void Mixer::getOutput(int16_t in1, int16_t in2, int16_t in3, int16_t in4, int16_t &outLeft, int16_t &outRight) {
    
    #define applyVolume(var, volume) do { \
        switch (volume) { \
            case 0x0: \
                var >>= 3; \
                break; \
            case 0x1: \
                var >>= 2; \
                break; \
            case 0x2: \
                var = (var * 3) >> 3; \
                break; \
            case 0x3: \
                var >>= 1; \
                break; \
            case 0x4: \
                var = (static_cast<int32_t>(var) * 5) >> 3; \
                break; \
            case 0x5: \
                var = (var * 3) >> 2; \
                break; \
            case 0x6: \
                var = (static_cast<int32_t>(var) * 7) >> 3; \
                break; \
            case 0x7: \
                break; \
        } } while (false)
    
    int16_t left = 0, right = 0;
    if (s01enable) {
        if (outputStat & Gbs::OUT_LEFT1) {
            left += in1;
        }
        if (outputStat & Gbs::OUT_LEFT2) {
            left += in2;
        }
        if (outputStat & Gbs::OUT_LEFT3) {
            left += in3;
        }
        if (outputStat & Gbs::OUT_LEFT4) {
            left += in4;
        }

        applyVolume(left, s01vol);
    }
    if (s02enable) {
        if (outputStat & Gbs::OUT_RIGHT1) {
            right += in1;
        }
        if (outputStat & Gbs::OUT_RIGHT2) {
            right += in2;
        }
        if (outputStat & Gbs::OUT_RIGHT3) {
            right += in3;
        }
        if (outputStat & Gbs::OUT_RIGHT4) {
            right += in4;
        }

        applyVolume(right, s02vol);
    }

    outLeft = left;
    outRight = right;
}

void Mixer::setEnable(Gbs::OutputFlags flags) {
    outputStat = flags;
}

void Mixer::setEnable(ChType ch, Gbs::Terminal term, bool enabled) {
    uint8_t flag = 0;
    if (term & Gbs::TERM_LEFT) {
        flag = 1 << static_cast<uint8_t>(ch);
    }

    if (term & Gbs::TERM_RIGHT) {
        flag |= 16 << static_cast<uint8_t>(ch);
    }

    if (enabled) {
        outputStat |= flag;
    } else {
        outputStat &= ~flag;
    }
}

void Mixer::setTerminalEnable(Gbs::Terminal term, bool enabled) {
    if (term & Gbs::TERM_LEFT) {
        s01enable = enabled;
    }

    if (term & Gbs::TERM_RIGHT) {
        s02enable = enabled;
    }
}

void Mixer::setTerminalVolume(Gbs::Terminal term, uint8_t volume) {
    if (volume > Gbs::MAX_TERM_VOLUME) {
        volume = Gbs::MAX_TERM_VOLUME;
    }

    if (term & Gbs::TERM_LEFT) {
        s01vol = volume;
    }

    if (term & Gbs::TERM_RIGHT) {
        s02vol = volume;
    }
}

}