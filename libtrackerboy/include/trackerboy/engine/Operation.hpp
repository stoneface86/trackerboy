
#pragma once

#include "trackerboy/data/TrackRow.hpp"
#include "trackerboy/trackerboy.hpp"

#include <cstdint>
#include <optional>

namespace trackerboy {


//
// Data structure representing a row operation to be executed.
//
// C-4 00 ... ... ... => { .note = 24, .instrument = 0, 
//
struct Operation {

    enum class PatternCommand : uint8_t {
        none,       // do nothing
        next,       // go to the next pattern in the order
        jump        // jump to the given pattern in patternCommandParam
    };

    enum class FrequencyMod : uint8_t {
        none,               // no frequency modulation
        portamento,         // automatic note slide
        pitchSlideUp,       // frequency slides toward a target
        pitchSlideDown,     // frequency slides toward a target
        noteSlideUp,        // frequency slides toward a target note
        noteSlideDown,
        arpeggio            // frequency alternates between 3 notes
    };

    Operation();

    // Create an operation from the given track row
    Operation(TrackRow const& row);

    // pattern effects / global effects
    PatternCommand patternCommand;
    uint8_t patternCommandParam;
    uint8_t speed; // change speed if nonzero
    bool halt;

    // note index
    // NOTE_CUT sets duration to 0, leaving note unset
    std::optional<uint8_t> note;

    // instrument
    std::optional<uint8_t> instrument;

    // track effects
    uint8_t delay;                   // delay this operation by the given number of frames
    std::optional<uint8_t> duration; // if set, cut the note in the given number of frames

    std::optional<uint8_t> envelope;
    std::optional<uint8_t> timbre;
    std::optional<uint8_t> panning;
    std::optional<uint8_t> sweep;

    // frequency effects
    FrequencyMod modulationType;
    uint8_t modulationParam;
    std::optional<uint8_t> vibrato;
    std::optional<uint8_t> vibratoDelay;
    std::optional<uint8_t> tune;
};


}
