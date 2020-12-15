
#pragma once

#include "Miniaudio.hpp"

#include <QFlags>

//
// Class containing application settings. Settings are modified via the ConfigDialog
// Signals are emitted when a setting changes.
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

        CategoryAll = CategorySound
    };
    Q_DECLARE_FLAGS(Categories, Category);

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

    Sound const& sound();


private:

    void setDevice(int index);

    void setSamplerate(int index);

    Miniaudio &mMiniaudio;

    Sound mSound;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Config::Categories);
