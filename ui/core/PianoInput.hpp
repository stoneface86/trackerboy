
#pragma once

#include "trackerboy/note.hpp"

#include <QHash>

#include <array>
#include <optional>

//
// Converts keyboard input -> trackerboy note. Used by PianoWidget and PatternEditor
// for getting a note input from the keyboard.
//
class PianoInput {

public:

    // 33 possible bindings

    // octave 0: base octave
    //        1: base octave + 1
    //        2: base octave + 2


    // OpenMPT
    // 1 2 3 4 5 6 7 8 9 0 - +  => ... ... ... ... ... ... ... ... ... ... ... ...
    // Q W E R T Y U I O P [ ]  => C-2 C#2 D-2 D#2 E-2 F-2 F#2 G-2 G#2 A-2 A#2 B-2
    // A S D F G H J K L ; ' \  => C-1 C#1 D-1 D#1 E-1 F-1 F#1 G-1 G#1 A-1 A#1 B-1
    // Z X C V B N M , . /      => C-0 C#0 D-0 D#0 E-0 F-0 F#0 G-0 G#0 A-0

    // FamiTracker
    // 1 2 3 4 5 6 7 8 9 0 - +  => ... C#1 D#1 ... F#1 G#1 A#1 ... C#2 D#2 ... F#2
    // Q W E R T Y U I O P [ ]  => C-1 D-1 E-1 F-1 G-1 A-1 B-1 C-2 D-2 E-2 F-2 G-2
    // A S D F G H J K L ; ' \  => ... C#0 D#0 ... F#0 G#0 A#0 ... C#1 D#1 ... ...
    // Z X C V B N M , . /      => C-0 D-0 E-0 F-0 G-0 A-0 B-0 C-1 D-1 E-1

    // 3 octaves of bindings
    static constexpr int TOTAL_BINDINGS = 36;

    using Bindings = std::array<int, TOTAL_BINDINGS>;

    explicit PianoInput();

    //
    // Copies the bindings of the given input object. This operation only copies bindings,
    // the base octave is left unchanged.
    //
    PianoInput& operator=(PianoInput const& rhs);

    Bindings const& bindings() const;

    void bind(int key, int semitoneOffset);

    void unbind(int semitone);

    //
    // Converts a Qt keycode to a trackerboy note. If no binding was set for
    // the key, nullopt is returned.
    //
    std::optional<trackerboy::Note> keyToNote(int key);

    void setOctave(int octave);

private:

    // base octave
    int mOctave;

    // array of key bindings, with the index being the semitone it is bound to
    Bindings mBindings;

    // map of keys -> semitone offsets
    // example: binding Key_Q to 12 means that Q is note C at mOctave + 1
    QHash<int, int> mBindingLookup;


};
