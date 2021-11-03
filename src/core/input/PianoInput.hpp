
#pragma once

#include "trackerboy/note.hpp"

#include <QHash>
class QSettings;

#include <array>
#include <optional>

#include <QCoreApplication>

//
// Converts keyboard input -> trackerboy note. Used by PianoWidget and PatternEditor
// for getting a note input from the keyboard.
//
class PianoInput {

    Q_DECLARE_TR_FUNCTIONS(PianoInput)

public:

    enum KeyboardLayout {
        // US QWERTY layout
        LayoutQwerty,
        LayoutQwertz,
        // French AZERTY layout
        LayoutAzerty,
        // Custom keybindings
        LayoutCustom,

        LayoutCount

    };

    // 2 octaves of note binds per keyboard row
    static constexpr int BindsPerRow = 24;

    //
    // Indices in the Bindings array for each key bind.
    //
    enum Binding {
        BindingNoteLower = 0,           // 0-23 are the lower row binds (0 is C+0, 12 is C+1, etc)
        BindingNoteUpper = BindsPerRow,  // 24-47 are the upper row binds (24 is C+1, 36 is C+2, etc)
        BindingNoteCut = BindsPerRow * 2,

        BindingCount
    };

    // upper and lower refer to the rows on a standard 104-key keyboard
    // the upper row is QWERTY and 123456
    // the lower row is ZXCVBN and ASDFGH
    //
    // by default, natural notes are mapped to the QWERTY and ZXCVBN rows,
    // and accidentals are mapped to the 123456 and ASDFGH rows

    //
    // The key binds are stored in an array of Qt keycodes. The index determines what the
    // key is bound to. 0-23 are note binds for octaves 0 and 1. 24-47 are note binds for
    // octaves 1 and 2 (octave 1 is repeated so that you can have multiple binds for this octave).
    // 48 and up are special binds, just note cut for now.
    //
    using Bindings = std::array<Qt::Key, BindingCount>;

    static constexpr auto NoKey = (Qt::Key)0;

    //
    // Get a human friendly name of the given layout
    //
    static QString layoutName(KeyboardLayout layout);

    explicit PianoInput();

    //
    // Get the current keyboard layout in use
    //
    KeyboardLayout layout() const;

    //
    // Change the current layout
    //
    void setLayout(KeyboardLayout layout);

    //
    // Get the current key bindings
    //
    Bindings const& bindings() const;

    //
    // Sets the key bindings from the given bindings. The layout is
    // then set to LayoutCustom.
    //
    void setBindings(Bindings const& bindings);

    //
    // Converts a Qt keycode to a trackerboy note. If no binding was set for
    // the key, nullopt is returned.
    //
    std::optional<trackerboy::Note> keyToNote(int key) const;

    //
    // The base octave used when converting a Qt::Key to a note
    //
    int octave() const;

    //
    // Set the base octave. Octave must be in range of 2-8.
    //
    void setOctave(int octave);

    //
    // Sets keybindings and layout from the given QSettings.
    //
    void readSettings(QSettings &settings);

    //
    // Writes current layout and keybindings to the given QSettings. Keybindings are
    // only written if the layout is LayoutCustom.
    //
    void writeSettings(QSettings &settings) const;

private:

    void mapBindings();

    // special semitones start at this offset
    // semitones below this offset are mapped to a note
    static constexpr int SPECIAL_START = 36;

    // base octave
    int mOctave;

    KeyboardLayout mLayout;
    Bindings mBindings;

    // map of keys -> semitone offsets
    // example: binding Key_Q to 12 means that Q is note C at mOctave + 1
    QHash<int, int> mBindingLookup;


};
