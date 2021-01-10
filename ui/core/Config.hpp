
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

        CategoryAll = CategorySound | CategoryAppearance | CategoryKeyboard
    };
    Q_DECLARE_FLAGS(Categories, Category);

    struct Appearance {
        ColorTable colors;
        QFont font;
        bool showFlats;         // if true flats will be shown for accidental notes
        bool showPreviews;      // if true, pattern previews will be rendered
    };

    struct Keyboard {

        PianoInput pianoInput;
    };

    struct Sound {
        int deviceIndex;
        unsigned samplerateIndex;   // index of the current samplerate
        unsigned buffersize;        // Number of frames to buffer when rendering
        int volume;                 // Synth volume level, percentage, 0-100
        bool lowLatency;            // low latency playback enable

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

    Appearance const& appearance();

    Keyboard const& keyboard();

    Sound const& sound();


private:

    void readColor(QSettings &settings, Color color, QColor def);

    void readPianoBinding(QSettings &settings, int semitone, Qt::Key def);

    Miniaudio &mMiniaudio;
    
    Appearance mAppearance;
    Keyboard mKeyboard;
    Sound mSound;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Config::Categories);
