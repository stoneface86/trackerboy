
#pragma once

#include "core/PianoInputBindings.hpp"

#include "trackerboy/note.hpp"

#include <QHash>
class QSettings;

#include <array>
#include <optional>

//
// Converts keyboard input -> trackerboy note. Used by PianoWidget and PatternEditor
// for getting a note input from the keyboard.
//
class PianoInput {

public:

    enum KeyboardLayout {
        // determine layout from System's locale
        LayoutSystem,
        // US QWERTY layout
        LayoutQwerty,
        LayoutQwertz,
        // French AZERTY layout
        LayoutAzerty,
        // Custom keybindings
        LayoutCustom,

        LayoutCount

    };

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
    PianoInputBindings const& bindings() const;

    //
    // Sets the key bindings from the given bindings. The layout is
    // then set to LayoutCustom.
    //
    void setBindings(PianoInputBindings const& bindings);

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
    PianoInputBindings mBindings;

    // map of keys -> semitone offsets
    // example: binding Key_Q to 12 means that Q is note C at mOctave + 1
    QHash<int, int> mBindingLookup;


};
