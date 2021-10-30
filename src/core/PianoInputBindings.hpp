
#pragma once

#include <Qt>

#include <array>


//
// POD struct of keybinds for PianoInput
//
struct PianoInputBindings {


    //
    // Special bindings. These keys do not bind to a note.
    // Currently the only special note in trackerboy is a note cut.
    //
    enum Specials {
        NoteCut,

        SpecialCount
    };

    static constexpr auto NoKey = (Qt::Key)0;
    static constexpr int BindsPerRow = 24;

    using RowBindings = std::array<Qt::Key, BindsPerRow>;
    using SpecialBindings = std::array<Qt::Key, SpecialCount>;

    // maps notes from octaves 0-1 from the base octave (ZXCVBN & ASDFGH)
    RowBindings lowerBindings;
    // maps notes from octaves 1-2 from the base octave (QWERTY & 123456)
    RowBindings upperBindings;
    SpecialBindings specials;

};

