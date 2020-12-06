
#pragma once

#include "Miniaudio.hpp"

#include <QObject>
#include <QSettings>

#include "trackerboy/ChType.hpp"

//
// Class containing application settings. Settings are modified via the ConfigDialog
// Signals are emitted when a setting changes.
//
class Config : public QObject {

    Q_OBJECT

    friend class ConfigDialog;

public:

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

    Config(Miniaudio &miniaudio, QObject *parent = nullptr);
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

signals:
    void soundConfigChanged();


private:

    void setDevice(int index);

    // an index of 0 is the device default
    void setSamplerate(int index);

    Miniaudio &mMiniaudio;

    Sound mSound;

    // just emits soundConfigChanged
    void applySound();

    //void setDevice(int backendIndex, int deviceIndex);

};

