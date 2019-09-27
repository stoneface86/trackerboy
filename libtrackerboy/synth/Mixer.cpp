
#include "trackerboy/synth/Mixer.hpp"

// each channel has a maximum volume of 0.2, so maximum volume of all channels is 0.8
#define VOL_MULTIPLIER 0.2f

namespace trackerboy {

static const float VOLUME_TABLE[8] = {
    0.125f,
    0.25f,
    0.375f,
    0.5f,
    0.625f,
    0.75f,
    0.875f,
    1.0f,
};


Mixer::Mixer() :
    s01enable(Gbs::DEFAULT_TERM_ENABLE),
    s02enable(Gbs::DEFAULT_TERM_ENABLE),
    s01vol(Gbs::DEFAULT_TERM_VOLUME),
    s02vol(Gbs::DEFAULT_TERM_VOLUME),
    outputStat(Gbs::OUT_OFF)
{
}

void Mixer::getOutput(float in1, float in2, float in3, float in4, float &outLeft, float &outRight) {
    float left = 0.0f, right = 0.0f;
    if (s01enable) {
        if (outputStat & Gbs::OUT_LEFT1) {
            left += in1 * VOL_MULTIPLIER;
        }
        if (outputStat & Gbs::OUT_LEFT2) {
            left += in2 * VOL_MULTIPLIER;
        }
        if (outputStat & Gbs::OUT_LEFT3) {
            left += in3 * VOL_MULTIPLIER;
        }
        if (outputStat & Gbs::OUT_LEFT4) {
            left += in4 * VOL_MULTIPLIER;
        }
    }
    if (s02enable) {
        if (outputStat & Gbs::OUT_RIGHT1) {
            right += in1 * VOL_MULTIPLIER;
        }
        if (outputStat & Gbs::OUT_RIGHT2) {
            right += in2 * VOL_MULTIPLIER;
        }
        if (outputStat & Gbs::OUT_RIGHT3) {
            right += in3 * VOL_MULTIPLIER;
        }
        if (outputStat & Gbs::OUT_RIGHT4) {
            right += in4 * VOL_MULTIPLIER;
        }
    }
    // TODO: cache these table lookups in a member variable
    outLeft = left * VOLUME_TABLE[s01vol];
    outRight = right * VOLUME_TABLE[s02vol];
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
        flag |= flag << 4;
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