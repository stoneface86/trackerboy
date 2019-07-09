
#include "gbsynth.hpp"

#include "tables.h"

// each channel has a maximum volume of 0.2, so maximum volume of all channels is 0.8
#define VOL_MULTIPLIER 0.2f

namespace gbsynth {

    Mixer::Mixer() :
        s01enable(DEFAULT_TERM_ENABLE),
        s02enable(DEFAULT_TERM_ENABLE),
        s01vol(DEFAULT_TERM_VOLUME),
        s02vol(DEFAULT_TERM_VOLUME),
        outputStat((OutputFlags)0)
    {
    }

    void Mixer::getOutput(float in1, float in2, float in3, float in4, float &outLeft, float &outRight) {
        float left = 0.0f, right = 0.0f;
        if (s01enable) {
            if (outputStat & OUT_SOUND1_LEFT) {
                left += in1 * VOL_MULTIPLIER;
            }
            if (outputStat & OUT_SOUND2_LEFT) {
                left += in2 * VOL_MULTIPLIER;
            }
            if (outputStat & OUT_SOUND3_LEFT) {
                left += in3 * VOL_MULTIPLIER;
            }
            if (outputStat & OUT_SOUND4_LEFT) {
                left += in4 * VOL_MULTIPLIER;
            }
        }
        if (s02enable) {
            if (outputStat & OUT_SOUND1_RIGHT) {
                right += in1 * VOL_MULTIPLIER;
            }
            if (outputStat & OUT_SOUND2_RIGHT) {
                right += in2 * VOL_MULTIPLIER;
            }
            if (outputStat & OUT_SOUND3_RIGHT) {
                right += in3 * VOL_MULTIPLIER;
            }
            if (outputStat & OUT_SOUND4_RIGHT) {
                right += in4 * VOL_MULTIPLIER;
            }
        }
        outLeft = left * VOLUME_TABLE[s01vol];
        outRight = right * VOLUME_TABLE[s02vol];
    }

    void Mixer::setTerminalEnable(Terminal term, bool enabled) {
        if (term == TERM_S01) {
            s01enable = enabled;
        } else {
            s02enable = enabled;
        }
    }

    void Mixer::setTerminalVolume(Terminal term, uint8_t volume) {
        if (volume > MAX_VOLUME) {
            volume = MAX_VOLUME;
        }
        if (term == TERM_S01) {
            s01vol = volume;
        } else {
            s02vol = volume;
        }
    }

    void Mixer::setEnable(OutputFlags flags) {
        outputStat = flags;
    }

    void Mixer::setEnable(ChType ch, Terminal term, bool enabled) {
        uint8_t flag = 1 << ch;
        if (term == TERM_S02) {
            flag <<= 4;
        }

        if (enabled) {
            outputStat |= flag;
        } else {
            outputStat &= ~flag;
        }
    }


}