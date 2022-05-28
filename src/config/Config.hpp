
#pragma once

#include "audio/AudioEnumerator.hpp"
#include "config/data/FontTable.hpp"
#include "config/data/GeneralConfig.hpp"
#include "config/data/MidiConfig.hpp"
#include "config/data/SoundConfig.hpp"
#include "config/data/ShortcutTable.hpp"
#include "midi/MidiEnumerator.hpp"

#include "config/data/PianoInput.hpp"
#include "config/data/Palette.hpp"

#include "verdigris/wobjectdefs.h"

//
// Class containing application settings. Settings are modified via the ConfigDialog
//
class Config {

public:

    //
    // Configuration categories flags. When applying settings, only the categories
    // with changes will get applied.
    //
    enum Category {
        CategoryNone = 0,
        CategorySound = 1,
        CategoryAppearance = 2,
        CategoryKeyboard = 4,
        CategoryMidi = 8,
        CategoryGeneral = 16,

        CategoryAll = CategorySound |
                      CategoryAppearance |
                      CategoryKeyboard |
                      CategoryMidi |
                      CategoryGeneral
    };
    W_DECLARE_FLAGS(Categories, Category)
    
    Config();
    ~Config() = default;

    //
    // Read the configuration settings from the given QSettings. Should be
    // called once on application start up
    //
    void readSettings(AudioEnumerator &audio, MidiEnumerator &midi);

    //
    // Write the current configuration settings to the given QSettings. Called
    // when MainWindow closes.
    //
    void writeSettings(AudioEnumerator const& audio, MidiEnumerator const& midi);

    FontTable& fonts();
    FontTable const& fonts() const;

    GeneralConfig& general();
    GeneralConfig const& general() const;

    MidiConfig& midi();
    MidiConfig const& midi() const;

    SoundConfig& sound();
    SoundConfig const& sound() const;

    Palette& palette();
    Palette const& palette() const;

    PianoInput& pianoInput();
    PianoInput const& pianoInput() const;

    ShortcutTable& shortcuts();
    ShortcutTable const& shortcuts() const;

private:
    
    FontTable mFonts;
    GeneralConfig mGeneral;
    MidiConfig mMidi;
    SoundConfig mSound;

    Palette mPalette;
    PianoInput mPianoInput;
    ShortcutTable mShortcuts;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Config::Categories)
W_REGISTER_ARGTYPE(Config::Categories)
W_REGISTER_ARGTYPE(Config::Category)
