
#pragma once

#include "core/ColorTable.hpp"
#include "core/Miniaudio.hpp"
#include "core/PianoInput.hpp"

#include <QFlags>
#include <QFont>
#include <QSettings>

//
// Class containing application settings. Settings are modified via the ConfigDialog
//
class Config {

    // only ConfigDialog can modify settings
    friend class ConfigDialog;
    friend class SoundConfigTab;

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

    struct Appearance {
        ColorTable colors;
        QFont font;
        bool showFlats;         // if true flats will be shown for accidental notes
        bool showPreviews;      // if true, pattern previews will be rendered
    };

    struct General {
        unsigned historyLimit;  // number of actions in the undo history, 0 for infinite
    };

    struct Keyboard {

        PianoInput pianoInput;
    };

    struct Midi {
        bool enabled;
        int backendIndex;
        int portIndex;
    };

    struct Sound {
        int deviceIndex;
        unsigned samplerateIndex;   // index of the current samplerate
        int latency;                // latency, or internal buffer size, in milliseconds
        int period;                 // period, in milliseconds
        int quality;                // synthesizer quality setting
    };

    Config(Miniaudio &miniaudio);
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

    Appearance const& appearance() const;

    General const& general() const;

    Keyboard const& keyboard() const;

    Midi const& midi() const;

    Sound const& sound() const;

    //
    // Disables MIDI configuration. This is to be called when a MIDI error occurs with
    // the configured device.
    //
    void disableMidi();


private:

    void readColor(QSettings &settings, Color color, QColor def);

    void readPianoBinding(QSettings &settings, int semitone, Qt::Key def);

    Miniaudio &mMiniaudio;
    
    Appearance mAppearance;
    General mGeneral;
    Keyboard mKeyboard;
    Midi mMidi;
    Sound mSound;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Config::Categories)
