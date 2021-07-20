
#pragma once

#include "core/ColorTable.hpp"
#include "core/PianoInput.hpp"

#include "gbapu.hpp"

#include <QFlags>
#include <QFont>
#include <QSettings>

//
// Class containing application settings. Settings are modified via the ConfigDialog
//
class Config {

    // only ConfigDialog can modify settings
    friend class ConfigDialog;

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
        static constexpr auto DEFAULT_FONT_FAMILY = "Cascadia Mono";
        static constexpr int DEFAULT_FONT_SIZE = 12;
        static constexpr bool DEFAULT_SHOW_FLATS = false;
        static constexpr bool DEFAULT_SHOW_PREVIEWS = true;

        ColorTable colors;
        QFont font;
        bool showFlats;         // if true flats will be shown for accidental notes
        bool showPreviews;      // if true, pattern previews will be rendered
    };

    struct General {
        static constexpr int MAX_HISTORY = 10000;
        static constexpr int MIN_HISTORY = 0;
        static constexpr int DEFAULT_HISTORY = 64;

        int historyLimit;  // number of actions in the undo history, 0 for infinite
    };

    struct Keyboard {

        PianoInput pianoInput;
    };

    struct Midi {
        static constexpr int DEFAULT_ENABLED = false;

        bool enabled;
        int backendIndex;
        int portIndex;
    };

    struct Sound {
        static constexpr int DEFAULT_PERIOD = 5;
        static constexpr int MIN_PERIOD = 1;
        static constexpr int MAX_PERIOD = 100;

        static constexpr int DEFAULT_LATENCY = 40;
        static constexpr int MIN_LATENCY = 1;
        static constexpr int MAX_LATENCY = 500;

        static constexpr int DEFAULT_SAMPLERATE = 4; // 44100 Hz
        
        static constexpr int DEFAULT_QUALITY = (int)gbapu::Apu::Quality::medium;


        int backendIndex;           // backend index in AudioProber list (-1 for no backend)
        int deviceIndex;            // device index from AudioProber device list
        int samplerateIndex;        // index of the current samplerate
        int latency;                // latency, or internal buffer size, in milliseconds
        int period;                 // period, in milliseconds
        int quality;                // synthesizer quality setting
    };

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

    
    Appearance mAppearance;
    General mGeneral;
    Keyboard mKeyboard;
    Midi mMidi;
    Sound mSound;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Config::Categories)
