
#pragma once

#include <cstdint>
#include <cstddef>

namespace trackerboy {

class FrequencyControl {

public:

    enum class SlideDirection {
        up,
        down
    };

    FrequencyControl() noexcept;

    uint16_t frequency() const noexcept;

    // used for testing

    /*bool slideActive() const noexcept;
    bool portamentoActive() const noexcept;
    bool arpeggioActive() const noexcept;*/


    // Effects 1xx and 2xx
    void setPitchSlide(SlideDirection dir, uint8_t param) noexcept;

    // Effects Qxy and Rxy
    void setNoteSlide(SlideDirection dir, uint8_t parameter) noexcept;

    // Effect 4xy
    void setVibrato(uint8_t vibrato) noexcept;

    // Effect 0xy
    void setArpeggio(uint8_t param) noexcept;

    // Effect 3xx
    void setPortamento(uint8_t param) noexcept;

    void setNote(uint8_t note) noexcept;

    // Effect Pxx
    void setTune(uint8_t param) noexcept;

    void apply() noexcept;

    void step() noexcept;

private:




    enum EffectCommand {
        EFF_ARP = 0,
        EFF_PITCH = 1,
        EFF_NOTE = 2,
        EFF_PORTAMENTO = 3
    };

    enum class ModType {
        none,               // no frequency modulation
        slide,              // frequency slides toward a target
        arpeggio            // frequency alternates between 3 notes
    };

    void setEffect(EffectCommand cmd, SlideDirection dir, uint8_t param) noexcept;
    void setTarget(uint16_t freq) noexcept;
    void setChord() noexcept;

    static constexpr size_t CHORD_LEN = 3;

    static constexpr uint8_t VIBRATO_PERIOD_SIZE = 64;
    static constexpr uint8_t VIBRATO_MASK = VIBRATO_PERIOD_SIZE - 1;
    static constexpr uint8_t VIBRATO_HALF_MASK = VIBRATO_PERIOD_SIZE / 2 - 1;
    static_assert((VIBRATO_PERIOD_SIZE & VIBRATO_MASK) == 0);

    static constexpr size_t VIBRATO_TABLE_EXTENTS = 16;
    static constexpr size_t VIBRATO_TABLE_LEN = VIBRATO_PERIOD_SIZE / 4;
    static const int8_t VIBRATO_TABLE[VIBRATO_TABLE_EXTENTS][VIBRATO_TABLE_LEN];

    static constexpr int FLAG_EFFECT_CMD = 0x3;
    static constexpr int FLAG_EFFECT_DIR = 0x4;
    static constexpr int FLAG_EFFECT_SET = 0x8;
    static constexpr int FLAG_NOTE_SET = 0x10;
    static constexpr int FLAG_PORTAMENTO = 0x20;
    static constexpr int FLAG_VIBRATO = 0x40;
    static constexpr int FLAG_SLIDING = 0x80;

    // only 1 slide can be active note/pitch/portamento
    // arpeggio cannot be used with slides
    // (setting a slide disables arpeggio, setting arpeggio disables a slide)

    int mFlags;
    uint8_t mEffectParam;
    ModType mMod;

    uint8_t mNote;

    int8_t mTune;
    int16_t mFrequency;

    // pitch slide

    uint8_t mSlideAmount;
    uint16_t mSlideTarget;


    // arpeggio

    uint8_t mChordParam;

    // index in the chord array
    unsigned mChordIndex;
    // note frequencies for the arpeggio "chord"
    uint16_t mChord[CHORD_LEN];

    // vibrato
    int8_t mVibratoCounter;
    uint8_t mVibratoIndex;
    uint8_t mVibratoSpeed;
    int8_t const *mVibratoTable;

};



}
