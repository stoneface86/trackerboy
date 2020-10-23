
#pragma once

#include "audio.hpp"

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
        int backendIndex;
        int deviceIndex;
        struct SoundIo *soundio;
        struct SoundIoDevice *device;
        audio::Samplerate samplerate;

        unsigned buffersize;   // Buffer size of playback queue in milleseconds
        unsigned volume;       // Master volume of playback queue output, 0-100

        unsigned bassFrequency;    // Bass cutoff frequency
        int treble;                // Treble amount, in dB
        unsigned trebleFrequency;  // Treble cutoff frequency


    };

    Config(audio::BackendTable &backendTable);

    //
    // Read the configuration settings from the given QSettings. Should be
    // called once on application start up
    //
    void readSettings(QSettings &settings);

    //
    // Write the current configuration settings to the given QSettings. Called
    // when MainWindow closes.
    //
    void writeSettings(QSettings &settings);

    Sound const& sound();

signals:
    void soundConfigChanged();


private:
    audio::BackendTable &mBackendTable;

    Sound mSound;

    // just emits soundConfigChanged
    void applySound();

    void setDevice(int backendIndex, int deviceIndex);

};

