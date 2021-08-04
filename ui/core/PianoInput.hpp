
#pragma once

#include "trackerboy/note.hpp"

#include <QHash>
#include <QSettings>

#include <array>
#include <optional>

//
// Converts keyboard input -> trackerboy note. Used by PianoWidget and PatternEditor
// for getting a note input from the keyboard.
//
class PianoInput {

public:

    enum KeyboardLayout {
        LayoutQwerty,
        LayoutQwertz,
        LayoutAzerty,
        LayoutCustom

    };

    // 3 octaves of bindings + note cut
    static constexpr int TOTAL_BINDINGS = 37;
    static constexpr int NOTE_CUT_BINDING = TOTAL_BINDINGS - 1;

    using Bindings = std::array<int, TOTAL_BINDINGS>;

    explicit PianoInput();

    //
    // Copies the bindings of the given input object. This operation only copies bindings,
    // the base octave is left unchanged.
    //
    PianoInput& operator=(PianoInput const& rhs);

    Bindings const& bindings() const;

    KeyboardLayout layout() const;

    //
    // Sets the default bindings for the given keyboard layout.
    //
    void setLayout(KeyboardLayout layout);

    //
    // Modifies a binding for the given semitone offset. The keyboard layout is
    // set to LayoutCustom after calling this function.
    //
    void bind(int key, int semitoneOffset);

    //
    // Removes an existing key binding for the given semitone offset
    //
    void unbind(int semitone);

    //
    // Converts a Qt keycode to a trackerboy note. If no binding was set for
    // the key, nullopt is returned.
    //
    std::optional<trackerboy::Note> keyToNote(int key) const;

    int octave() const;

    void setOctave(int octave);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    void clearBindings();

    // base octave
    int mOctave;

    KeyboardLayout mLayout;

    // array of key bindings, with the index being the semitone it is bound to
    Bindings mBindings;

    // map of keys -> semitone offsets
    // example: binding Key_Q to 12 means that Q is note C at mOctave + 1
    QHash<int, int> mBindingLookup;


};
