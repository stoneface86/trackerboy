
#pragma once

#include <cstdint>

#include "trackerboy/synth/Synth.hpp"


namespace trackerboy {


class NoteControl {

public:

    NoteControl(ChType trackId);

    uint16_t frequency() const noexcept;

    bool isPlaying() const noexcept;

    void noteTrigger(uint8_t note, uint8_t delay = 0);

    void noteCut(uint8_t delay = 0);

    void reset() noexcept;

    void step(Synth &synth) noexcept;

private:

    void cut(Synth &synth) noexcept;

    static constexpr int STAT_PLAYING = 0x1;
    static constexpr int STAT_TRIGGER = 0x2;
    static constexpr int STAT_CUT     = 0x4;

    ChType const mTrackId;

    int mStatus;

    uint8_t mTriggerCounter;    // frames until the note gets triggerred
    uint8_t mCutCounter;        // frames until the note gets cut

    uint8_t mNote;
    uint16_t mFrequency;

};


}
