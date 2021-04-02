
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/note.hpp"
#include "trackerboy/trackerboy.hpp"

#include <algorithm>
#include <optional>

namespace trackerboy {

// sine vibrato was removed to reduce memory usage for the driver. It was replaced with
// a simplier version (square vibrato).

FrequencyControl::FrequencyControl() noexcept :
    mNewNote(false),
    mEffectToApply(),
    mMod(ModType::none),
    mNote(0),
    mTune(0),
    mFrequency(0),
    mSlideAmount(0),
    mSlideTarget(0),
    mChordOffset1(0),
    mChordOffset2(0),
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
    if (mVibratoEnabled && mVibratoDelayCounter == 0) {
        freq += mVibratoValue;
    }

    return static_cast<uint16_t>(std::clamp(
        freq,
        static_cast<int16_t>(0),
        static_cast<int16_t>(GB_MAX_FREQUENCY))
        );

}

void FrequencyControl::reset() noexcept {
    mNewNote = false;
    mEffectToApply.reset();
    mMod = ModType::none;
    mNote = 0;
    mTune = 0;
    mFrequency = 0;
    mSlideAmount = 0;
    mSlideTarget = 0;
    mChordOffset1 = 0;
    mChordOffset2 = 0;
    mChordIndex = 0;
    mChord.fill(0);
    mVibratoDelayCounter = 0;
    mVibratoCounter = 0;
    mVibratoValue = 0;
    mVibratoDelay = 0;
    mVibratoParam = 0;
}


void FrequencyControl::setPitchSlide(SlideDirection dir, uint8_t param) noexcept {
    mEffectToApply.emplace(ModType::pitchSlide, param, dir);
}

void FrequencyControl::setNoteSlide(SlideDirection dir, uint8_t param) noexcept {
    mEffectToApply.emplace(ModType::noteSlide, param, dir);

}

void FrequencyControl::setVibrato(uint8_t param) noexcept {
    mVibratoParam = param;
    if (!(param & 0x0F)) {
        // extent is 0, disable vibrato
        mVibratoEnabled = false;
        mVibratoValue = 0;
    } else {
        // both nibbles are non-zero, set vibrato
        mVibratoEnabled = true;
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
    mEffectToApply.emplace(ModType::arpeggio, param);

}

void FrequencyControl::setPortamento(uint8_t param) noexcept {
    mEffectToApply.emplace(ModType::portamento, param);
}

void FrequencyControl::setNote(uint8_t note) noexcept {
    // ignore special note and illegal note indices
    if (note <= NOTE_LAST) {
        mNote = note;
        mNewNote = true;
    }
}

void FrequencyControl::setTune(uint8_t param) noexcept {
    // tune values have a bias of 0x80, so 0x80 is 0, is in tune
    // 0x81 is +1, frequency is pitch adjusted by 1
    // 0x7F is -1, frequency is pitch adjusted by -1
    mTune = static_cast<int8_t>(param - 0x80);
}

void FrequencyControl::apply() noexcept {

    // the arpeggio chord needs to be recalculated when:
    //  * a new note is triggerred and arpeggio is active
    //  * arpeggio effect is activated
    bool updateChord = false;

    // copy the current note (noteSlide effect changes it)
    auto noteCurr = mNote;
    if (mNewNote && mMod == ModType::noteSlide) {
        // setting a new note cancels a note slide
        mMod = ModType::none;
    }

    if (mEffectToApply) {
        auto param = mEffectToApply->parameter;
        switch (mEffectToApply->type) {
            case ModType::arpeggio:
                if (param == 0) {
                    mMod = ModType::none;
                } else {
                    mMod = ModType::arpeggio;
                    mChordOffset1 = param >> 4;
                    mChordOffset2 = param & 0xF;
                    updateChord = true;
                }
                break;
            case ModType::pitchSlide:
                if (param == 0) {
                    mMod = ModType::none;
                } else {
                    mMod = ModType::pitchSlide;
                    if (mEffectToApply->direction == SlideDirection::up) {
                        mSlideTarget = GB_MAX_FREQUENCY;
                    } else {
                        mSlideTarget = 0;
                    }
                    mSlideAmount = param;
                }
                break;
            case ModType::noteSlide:
                mSlideAmount = 1 + (2 * (param & 0xF));
                // upper 4 bits is the # of semitones to slide to
                {
                    uint8_t semitones = param >> 4;
                    uint8_t targetNote = mNote;
                    if (mEffectToApply->direction == SlideDirection::up) {
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
                    mMod = ModType::noteSlide;
                    mSlideTarget = NOTE_FREQ_TABLE[targetNote];
                    // current note becomes the target note (even though it hasn't reached it yet)
                    // this allows for bigger slides by chaining multiple note slide effects
                    mNote = targetNote;
                }
                break;
            case ModType::portamento:
                if (param == 0) {
                    // turn off portamento
                    mMod = ModType::none;
                } else {
                    mMod = ModType::portamento;
                    mSlideAmount = param;
                }
                break;
            default:
                // should never happen
                break;
        }
        mEffectToApply.reset();
    }

    if (mNewNote) {
        auto freq = NOTE_FREQ_TABLE[noteCurr];
        if (mMod == ModType::portamento) {
            mSlideTarget = freq;
        } else {
            if (mMod == ModType::arpeggio) {
                updateChord = true;
            }
            mFrequency = freq;
        }


        if (mVibratoEnabled) {
            mVibratoDelayCounter = mVibratoDelay;
            mVibratoCounter = 0;
            mVibratoValue = mVibratoParam & 0xF;
        }

        mNewNote = false;
    }

    if (updateChord) {
        setChord();
    }


}

void FrequencyControl::step() noexcept {

    if (mVibratoEnabled) {

        if (mVibratoDelayCounter) {
            --mVibratoDelayCounter;
        } else {
            if (mVibratoCounter == 0) {
                mVibratoValue = -mVibratoValue;
                mVibratoCounter = mVibratoParam >> 4;
            } else {
                --mVibratoCounter;
            }
        }
    }

    switch (mMod) {
        case ModType::none:
            break;
        case ModType::portamento:
        case ModType::pitchSlide:
        case ModType::noteSlide:
            if (mFrequency != mSlideTarget) {
                if (mFrequency < mSlideTarget) {
                    // sliding up
                    mFrequency += mSlideAmount;
                    if (mFrequency > mSlideTarget) {
                        finishSlide();
                    }
                } else {
                    // sliding down
                    mFrequency -= mSlideAmount;
                    if (mFrequency < mSlideTarget) {
                        finishSlide();
                    }
                }
            }
            break;
        case ModType::arpeggio:
            mFrequency = mChord[mChordIndex];
            if (++mChordIndex == mChord.size()) {
                mChordIndex = 0;
            }
            break;
    }



}

void FrequencyControl::setChord() noexcept {
    // first note in the chord is always the current note
    mChord[0] = NOTE_FREQ_TABLE[mNote];
    // second note is the upper nibble + the current (clamped to the last possible note)
    mChord[1] = NOTE_FREQ_TABLE[std::min(mNote + mChordOffset1, static_cast<int>(NOTE_LAST))];
    // third note is the lower nibble + current (also clamped)
    mChord[2] = NOTE_FREQ_TABLE[std::min(mNote + mChordOffset2, static_cast<int>(NOTE_LAST))];
}

void FrequencyControl::finishSlide() noexcept {
    mFrequency = mSlideTarget;
    if (mMod == ModType::noteSlide) {
        // stop sliding once the target note is reached
        mMod = ModType::none;
    }
}


}
