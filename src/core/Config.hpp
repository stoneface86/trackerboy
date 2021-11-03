
#pragma once

#include "core/config/AppearanceConfig.hpp"
#include "core/config/GeneralConfig.hpp"
#include "core/config/KeyboardConfig.hpp"
#include "core/config/MidiConfig.hpp"
#include "core/config/SoundConfig.hpp"

#include "core/input/PianoInput.hpp"
#include "core/Palette.hpp"

#include <QFlags>

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

        CategoryAll = CategorySound |
                      CategoryAppearance |
                      CategoryKeyboard |
                      CategoryMidi
    };
    Q_DECLARE_FLAGS(Categories, Category)
    
    Config();
    ~Config() = default;

    //
    // Read the configuration settings from the given QSettings. Should be
    // called once on application start up
    //
    void readSettings();

    //
    // Write the current configuration settings to the given QSettings. Called
    // when MainWindow closes.
    //
    void writeSettings();

    AppearanceConfig& appearance();
    AppearanceConfig const& appearance() const;

    GeneralConfig& general();
    GeneralConfig const& general() const;

    // KeyboardConfig& keyboard();
    // KeyboardConfig const& keyboard() const;

    MidiConfig& midi();
    MidiConfig const& midi() const;

    SoundConfig& sound();
    SoundConfig const& sound() const;

    Palette& palette();
    Palette const& palette() const;

    PianoInput& pianoInput();
    PianoInput const& pianoInput() const;

    //
    // Disables MIDI configuration. This is to be called when a MIDI error occurs with
    // the configured device.
    //
    void disableMidi();


private:

    
    AppearanceConfig mAppearance;
    GeneralConfig mGeneral;
    // KeyboardConfig mKeyboard;
    MidiConfig mMidi;
    SoundConfig mSound;

    Palette mPalette;
    PianoInput mPianoInput;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Config::Categories)
