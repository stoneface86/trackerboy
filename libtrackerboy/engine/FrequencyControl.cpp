
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/note.hpp"
#include "trackerboy/trackerboy.hpp"

#include <algorithm>
#include <optional>

namespace trackerboy {


FrequencyControl::Context::Context(Instrument const& instrument) :
    arpSequence(instrument.enumerateSequence(Instrument::SEQUENCE_ARP)),
    pitchSequence(instrument.enumerateSequence(Instrument::SEQUENCE_PITCH))
{
}

FrequencyControl::~FrequencyControl() {

}

FrequencyControl::FrequencyControl(uint16_t maxFrequency, uint8_t maxNote) noexcept :
    mMaxFrequency(maxFrequency),
    mMaxNote(maxNote),
    mMod(ModType::none),
    mNote(0),
    mTune(0),
    mFrequency(0),
    mSlideAmount(0),
    mSlideTarget(0),
    mInstrumentPitch(0),
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

    int16_t freq = mFrequency + mTune + mInstrumentPitch;

    // vibrato
    if (mVibratoEnabled && mVibratoDelayCounter == 0) {
        freq += mVibratoValue;
    }

    return static_cast<uint16_t>(std::clamp(
        freq,
        static_cast<int16_t>(0),
        static_cast<int16_t>(mMaxFrequency))
        );

}

void FrequencyControl::reset() noexcept {
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

void FrequencyControl::apply(Operation const& op) noexcept {

    // the arpeggio chord needs to be recalculated when:
    //  * a new note is triggerred and arpeggio is active
    //  * arpeggio effect is activated
    bool updateChord = false;

    bool newNote;
    if (op.note && *op.note <= mMaxNote) {
        if (mMod == ModType::noteSlide) {
            // setting a new note cancels a note slide
            mMod = ModType::none;
        }

        mNote = *op.note;
        newNote = true;
    } else {
        newNote = false;
    }
    uint8_t currNote = mNote;
    

    switch (op.modulationType) {
        case Operation::FrequencyMod::arpeggio:
            if (op.modulationParam == 0) {
                mMod = ModType::none;
            } else {
                mMod = ModType::arpeggio;
                mChordOffset1 = op.modulationParam >> 4;
                mChordOffset2 = op.modulationParam & 0xF;
                updateChord = true;
            }
            break;
        case Operation::FrequencyMod::pitchSlideDown:
        case Operation::FrequencyMod::pitchSlideUp:
            if (op.modulationParam == 0) {
                mMod = ModType::none;
            } else {
                mMod = ModType::pitchSlide;
                if (op.modulationType == Operation::FrequencyMod::pitchSlideUp) {
                    mSlideTarget = mMaxFrequency;
                } else {
                    mSlideTarget = 0;
                }
                mSlideAmount = op.modulationParam;
            }
            break;
        case Operation::FrequencyMod::noteSlideDown:
        case Operation::FrequencyMod::noteSlideUp:
            mSlideAmount = 1 + (2 * (op.modulationParam & 0xF));
            // upper 4 bits is the # of semitones to slide to
            {
                uint8_t semitones = op.modulationParam >> 4;
                uint8_t targetNote = mNote;
                if (op.modulationType == Operation::FrequencyMod::noteSlideUp) {
                    targetNote += semitones;
                    if (targetNote > mMaxNote) {
                        targetNote = mMaxNote; // clamp to highest note
                    }
                } else {
                    if (targetNote < semitones) {
                        targetNote = 0; // clamp to the lowest possible note
                    } else {
                        targetNote -= semitones;
                    }
                }
                mMod = ModType::noteSlide;
                mSlideTarget = noteLookup(targetNote);
                // current note becomes the target note (even though it hasn't reached it yet)
                // this allows for bigger slides by chaining multiple note slide effects
                mNote = targetNote;
            }
            break;
        case Operation::FrequencyMod::portamento:
            if (op.modulationParam == 0) {
                // turn off portamento
                mMod = ModType::none;
            } else {
                if (mMod != ModType::portamento) {
                    mSlideTarget = mFrequency;
                    mMod = ModType::portamento;
                }
                mSlideAmount = op.modulationParam;
            }
            break;
        default:
            // no mod type set, do nothing
            break;
    }

    if (op.vibrato) {
        auto param = *op.vibrato;
        mVibratoParam = param;
        if (!(param & 0x0F)) {
            // extent is 0, disable vibrato
            mVibratoEnabled = false;
            mVibratoValue = 0;
        } else {
            // extent is non-zero, set vibrato
            mVibratoEnabled = true;
            int8_t newvalue = param & 0xF;
            if (mVibratoValue < 0) {
                mVibratoValue = -newvalue;
            } else {
                mVibratoValue = newvalue;
            }
        }
    }

    if (op.vibratoDelay) {
        mVibratoDelay = *op.vibratoDelay;
    }

    if (op.tune) {
        // tune values have a bias of 0x80, so 0x80 is 0, is in tune
        // 0x81 is +1, frequency is pitch adjusted by 1
        // 0x7F is -1, frequency is pitch adjusted by -1
        mTune = (int8_t)(*op.tune - 0x80);
    }

    if (newNote) {
        auto freq = noteLookup(currNote);
        if (mMod == ModType::portamento) {
            // automatic portamento, slide to this note
            mSlideTarget = freq;
        } else {
            // otherwise set the current frequency
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

        mInstrumentPitch = 0;
    }

    if (updateChord) {
        // first note in the chord is always the current note
        mChord[0] = noteLookup(mNote);
        // second note is the upper nibble + the current (clamped to the last possible note)
        mChord[1] = noteLookup(std::min((uint8_t)(mNote + mChordOffset1), mMaxNote));
        // third note is the lower nibble + current (also clamped)
        mChord[2] = noteLookup(std::min((uint8_t)(mNote + mChordOffset2), mMaxNote));
    }


}

void FrequencyControl::useInstrument(Instrument const& instrument) noexcept {
    mContext.emplace(instrument);
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

    std::optional<uint8_t> arp;
    if (mContext) {
        auto pitch = mContext->pitchSequence.next();
        if (pitch) {
            mInstrumentPitch += (int8_t)*pitch;
        }

        arp = mContext->arpSequence.next();
    }

    if (arp) {
        // absolute
        int8_t offset = (int8_t)*arp;
        int8_t note = (int8_t)std::clamp((int)mNote + offset, 0, (int)mMaxNote);
        mFrequency = noteLookup(note);

        // relative (same as absolute but saves the note)
        //mNote = note;

        // fixed
        //mNote = *arp;
        //mFrequency = noteLookup(mNote);
    } else {
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



}

void FrequencyControl::finishSlide() noexcept {
    mFrequency = mSlideTarget;
    if (mMod == ModType::noteSlide) {
        // stop sliding once the target note is reached
        mMod = ModType::none;
    }
}


ToneFrequencyControl::ToneFrequencyControl() noexcept :
    FrequencyControl(GB_MAX_FREQUENCY, NOTE_LAST)
{
}

uint16_t ToneFrequencyControl::noteLookup(uint8_t note) {
    return NOTE_FREQ_TABLE[note];
}

// Noise "pitch" units
// frequency values for the noise channel are a modified version of the NR43 register, in
// that the step-width bit is removed
// sssswddd => ssssddd
//
// Noise pitches now range from 0 to 0x67.
// 0 is the highest period, which is not ideal, since 0 should be the lowest frequency possible
// so we use an adjust pitch value by calculating p = 0x67 - p;
//
//
// to convert pitch p to nr43
// nr43 = 0x67 - p
// nr43 = ((nr43 << 1) & 0xF0) | (nr43 & 7)


NoiseFrequencyControl::NoiseFrequencyControl() noexcept :
    FrequencyControl(NOTE_NOISE_LAST, NOTE_NOISE_LAST)
{
}

uint16_t NoiseFrequencyControl::noteLookup(uint8_t note) {
    return note;
}

uint8_t NoiseFrequencyControl::toNR43(uint16_t frequency) noexcept {
    return NOTE_NOISE_TABLE[frequency];

}

}
