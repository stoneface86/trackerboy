
#include "trackerboy/synth.hpp"

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
    s01enable(DEFAULT_TERM_ENABLE),
    s02enable(DEFAULT_TERM_ENABLE),
    s01vol(DEFAULT_TERM_VOLUME),
    s02vol(DEFAULT_TERM_VOLUME),
    outputStat(OutputFlags::all_off)
{
}

void Mixer::getOutput(float in1, float in2, float in3, float in4, float &outLeft, float &outRight) {
    float left = 0.0f, right = 0.0f;
    if (s01enable) {
        if ((outputStat & OutputFlags::left1) == OutputFlags::left1) {
            left += in1 * VOL_MULTIPLIER;
        }
        if ((outputStat & OutputFlags::left2) == OutputFlags::left2) {
            left += in2 * VOL_MULTIPLIER;
        }
        if ((outputStat & OutputFlags::left3) == OutputFlags::left3) {
            left += in3 * VOL_MULTIPLIER;
        }
        if ((outputStat & OutputFlags::left4) == OutputFlags::left4) {
            left += in4 * VOL_MULTIPLIER;
        }
    }
    if (s02enable) {
        if ((outputStat & OutputFlags::right1) == OutputFlags::right1) {
            right += in1 * VOL_MULTIPLIER;
        }
        if ((outputStat & OutputFlags::right2) == OutputFlags::right2) {
            right += in2 * VOL_MULTIPLIER;
        }
        if ((outputStat & OutputFlags::right3) == OutputFlags::right3) {
            right += in3 * VOL_MULTIPLIER;
        }
        if ((outputStat & OutputFlags::right4) == OutputFlags::right4) {
            right += in4 * VOL_MULTIPLIER;
        }
    }
    outLeft = left * VOLUME_TABLE[s01vol];
    outRight = right * VOLUME_TABLE[s02vol];
}

void Mixer::setEnable(OutputFlags flags) {
    outputStat = flags;
}

void Mixer::setEnable(ChType ch, Terminal term, bool enabled) {
    uint8_t flag = 1 << static_cast<uint8_t>(ch);
    if (term == Terminal::s02) {
        flag <<= 4;
    }

    if (enabled) {
        outputStat |= static_cast<OutputFlags>(flag);
    } else {
        outputStat &= ~static_cast<OutputFlags>(flag);
    }
}

void Mixer::setTerminalEnable(Terminal term, bool enabled) {
    if (term == Terminal::s01) {
        s01enable = enabled;
    } else {
        s02enable = enabled;
    }
}

void Mixer::setTerminalVolume(Terminal term, uint8_t volume) {
    if (volume > MAX_VOLUME) {
        volume = MAX_VOLUME;
    }
    if (term == Terminal::s01) {
        s01vol = volume;
    } else {
        s02vol = volume;
    }
}

}