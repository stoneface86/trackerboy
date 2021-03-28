
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/note.hpp"
#include "trackerboy/trackerboy.hpp"

#include <algorithm>


#define byteHasZeroNibble(byte) (!(byte & 0xF) || !(byte & 0xF0))

namespace trackerboy {

// sine vibrato was removed to reduce memory usage for the driver. It was replaced with
// a simplier version (square vibrato).





FrequencyControl::FrequencyControl() noexcept :
    mFlags(0),
    mMod(ModType::none),
    mNote(0),
    mTune(0),
    mFrequency(0),
    mSlideAmount(0),
    mSlideTarget(0),
    mChordParam(0),
    mChordIndex(0),
    mChord{ 0 },
    mVibratoDelayCounter(0),
    mVibratoCounter(0),
    mVibratoValue(0),
    mVibratoDelay(0),
    mVibratoParam(0)
{
}

uint16_t FrequencyControl::frequency() const noexcept {

    int16_t freq = mFrequency + mTune;

    // vibrato
    if (!!(mFlags & FLAG_VIBRATO) && mVibratoDelayCounter == 0) {
        freq += mVibratoValue;
    }

    return static_cast<uint16_t>(std::clamp(
        freq,
        static_cast<int16_t>(0),
        static_cast<int16_t>(GB_MAX_FREQUENCY))
        );

}

void FrequencyControl::reset() noexcept {
    mFlags = 0;
    mMod = ModType::none;
    mNote = 0;
    mTune = 0;
    mFrequency = 0;
    mSlideAmount = 0;
    mSlideTarget = 0;
    mChordParam = 0;
    mChordIndex = 0;
    mVibratoDelayCounter = 0;
    mVibratoCounter = 0;
    mVibratoValue = 0;
    mVibratoDelay = 0;
    mVibratoParam = 0;
}


void FrequencyControl::setPitchSlide(SlideDirection dir, uint8_t param) noexcept {
    setEffect(EFF_PITCH, dir, param);
}

void FrequencyControl::setNoteSlide(SlideDirection dir, uint8_t param) noexcept {
    setEffect(EFF_NOTE, dir, param);

}

void FrequencyControl::setVibrato(uint8_t param) noexcept {
    mVibratoParam = param;
    if (!(param & 0x0F)) {
        // extent is 0, disable vibrato
        mFlags &= ~FLAG_VIBRATO;
        mVibratoValue = 0;
    } else {
        // both nibbles are non-zero, set vibrato
        mFlags |= FLAG_VIBRATO;
        if (mVibratoValue) {
            int8_t newvalue = param & 0xF;
            if (mVibratoValue < 0) {
                mVibratoValue = -newvalue;
            } else {
                mVibratoValue = newvalue;
            }
        }
    }
}

void FrequencyControl::setVibratoDelay(uint8_t delay) noexcept {
    mVibratoDelay = delay;
}

void FrequencyControl::setArpeggio(uint8_t param) noexcept {

    setEffect(EFF_ARP, SlideDirection::down, param);

}

void FrequencyControl::setPortamento(uint8_t param) noexcept {
    setEffect(EFF_PORTAMENTO, SlideDirection::down, param);
}

void FrequencyControl::setNote(uint8_t note) noexcept {
    // ignore special note and illegal note indices
    if (note <= NOTE_LAST) {
        mFlags |= FLAG_NOTE_SET;
        mNote = note;
    }
}

void FrequencyControl::setTune(uint8_t param) noexcept {
    // tune values have a bias of 0x80, so 0x80 is 0, is in tune
    // 0x81 is +1, frequency is pitch adjusted by 1
    // 0x7F is -1, frequency is pitch adjusted by -1
    mTune = static_cast<int8_t>(param - 0x80);
}

void FrequencyControl::apply() noexcept {
    bool updateFreq = false;
    int16_t freq = mFrequency;

    if (!!(mFlags & FLAG_NOTE_SET)) {
        freq = NOTE_FREQ_TABLE[mNote];
        updateFreq = true;
        if (mMod == ModType::arpeggio) {
            setChord();
        } else if (!!(mFlags & FLAG_PORTAMENTO)) {
            setTarget(freq);
        }

        if (!!(mFlags & FLAG_VIBRATO)) {
            mVibratoDelayCounter = mVibratoDelay;
            mVibratoCounter = 0;
            mVibratoValue = mVibratoParam & 0xF;
        }

        mFlags &= ~FLAG_NOTE_SET;
    }

    mFlags &= ~FLAG_PORTAMENTO;

    if (!!(mFlags & FLAG_EFFECT_SET)) {
        EffectCommand cmd = static_cast<EffectCommand>(mFlags & FLAG_EFFECT_CMD);

        bool hasTarget = true;
        uint16_t target = 0;
        mFlags &= ~FLAG_PORTAMENTO;

        switch (cmd) {
            case EFF_ARP:
                if (mEffectParam == 0) {
                    mMod = ModType::none;
                } else {
                    mMod = ModType::arpeggio;
                    mChordParam = mEffectParam;
                    setChord();
                }
                hasTarget = false;
                break;
            case EFF_PITCH:
                if (mEffectParam == 0) {
                    mMod = ModType::none;
                    hasTarget = false;
                } else {
                    if (!!(mFlags & FLAG_EFFECT_DIR)) {
                        target = 0x7FF;
                    }
                    mSlideAmount = mEffectParam;
                }
                break;
            case EFF_NOTE:
                // lower 4 bits of param is the slide amount
                // the speed is determined by the formula 2x + 1
                // (1 to 31 pitch units/frame)
                mSlideAmount = 1 + (2 * (mEffectParam & 0xF));
                // upper 4 bits is the # of semitones to slide to
                {
                    uint8_t semitones = mEffectParam >> 4;
                    uint8_t targetNote = mNote;
                    if (!!(mFlags & FLAG_EFFECT_DIR)) {
                        targetNote += semitones;
                        if (targetNote > NOTE_LAST) {
                            targetNote = NOTE_LAST; // clamp to highest note
                        }
                    } else {
                        if (targetNote < semitones) {
                            targetNote = 0; // clamp to the lowest possible note
                        } else {
                            targetNote -= semitones;
                        }
                    }
                    target = NOTE_FREQ_TABLE[targetNote];
                    mNote = targetNote;
                }
                break;
            case EFF_PORTAMENTO:
                if (mEffectParam == 0) {
                    mMod = ModType::none;
                    hasTarget = false;
                } else {
                    mFlags |= FLAG_PORTAMENTO;
                    target = freq;
                    updateFreq = false;
                    mSlideAmount = mEffectParam;
                }
                break;
        }

        if (hasTarget) {
            setTarget(target);
        }

        mFlags &= ~(FLAG_EFFECT_CMD | FLAG_EFFECT_DIR | FLAG_EFFECT_SET);
    }

    if (updateFreq) {
        mFrequency = freq;
    }

}

void FrequencyControl::step() noexcept {

    if (!!(mFlags & FLAG_VIBRATO)) {
        
        if (mVibratoDelayCounter) {
            --mVibratoDelayCounter;
        } else {
            if (mVibratoCounter == 0) {
                mVibratoValue = -mVibratoValue;
                mVibratoCounter = mVibratoParam >> 4;
            } else {
                //mVibratoValue = 0;
                --mVibratoCounter;
            }
        }
    }

    switch (mMod) {
        case ModType::none:
            break;
        case ModType::slide:
            if (!!(mFlags & FLAG_SLIDING)) {
                if (mFrequency < mSlideTarget) {
                    // sliding up
                    mFrequency += mSlideAmount;
                    if (mFrequency > mSlideTarget) {
                        mFrequency = mSlideTarget;
                        mFlags &= ~FLAG_SLIDING;
                    }
                } else {
                    // sliding down
                    mFrequency -= mSlideAmount;
                    if (mFrequency < mSlideTarget) {
                        mFrequency = mSlideTarget;
                        mFlags &= ~FLAG_SLIDING;
                    }
                }
            }
            break;
        case ModType::arpeggio:
            mFrequency = mChord[mChordIndex];
            if (++mChordIndex == CHORD_LEN) {
                mChordIndex = 0;
            }
            break;
    }



}

void FrequencyControl::setTarget(uint16_t freq) noexcept {
    mMod = ModType::slide;
    mSlideTarget = freq;
    if (mFrequency != mSlideTarget) {
        mFlags |= FLAG_SLIDING;
    }
}

void FrequencyControl::setChord() noexcept {
    // first note in the chord is always the current note
    mChord[0] = NOTE_FREQ_TABLE[mNote];
    // second note is the upper nibble + the current (clamped to the last possible note)
    mChord[1] = NOTE_FREQ_TABLE[std::min(mNote + (mChordParam >> 4), static_cast<int>(NOTE_LAST))];
    // third note is the lower nibble + current (also clamped)
    mChord[2] = NOTE_FREQ_TABLE[std::min(mNote + (mChordParam & 0xF), static_cast<int>(NOTE_LAST))];
}

void FrequencyControl::setEffect(EffectCommand cmd, SlideDirection dir, uint8_t param) noexcept {
    mFlags = (mFlags & ~FLAG_EFFECT_CMD) | cmd | FLAG_EFFECT_SET;
    if (dir == SlideDirection::up) {
        mFlags |= FLAG_EFFECT_DIR;
    } else {
        mFlags &= ~FLAG_EFFECT_DIR;
    }
    mEffectParam = param;
}


}
