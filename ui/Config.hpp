
#pragma once

#include "ColorTable.hpp"
#include "Miniaudio.hpp"

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

        CategoryAll = CategorySound | CategoryAppearance
    };
    Q_DECLARE_FLAGS(Categories, Category);

    struct Appearance {
        ColorTable colors;
        QFont font;
        bool showFlats;         // if true flats will be shown for accidental notes
        bool showPreviews;      // if true, pattern previews will be rendered
    };

    struct Sound {

        // device settings
        //int backendIndex;
        ma_context *context;
        int deviceIndex;
        ma_device_id *device;

        unsigned samplerate;        // samplerate to use (0 for device default)
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

    Sound const& sound();


private:

    void setDevice(int index);

    void setSamplerate(int index);

    void readColor(QSettings &settings, Color color, QColor def);

    Miniaudio &mMiniaudio;
    
    Appearance mAppearance;
    Sound mSound;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Config::Categories);
