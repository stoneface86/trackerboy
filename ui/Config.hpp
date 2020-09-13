
#pragma once

#include <QObject>
#include <QSettings>

#include "trackerboy/ChType.hpp"

//
// Class containing application settings. Settings are modified via the ConfigDialog
// Signals are emitted when a setting changes.
//
class Config : public QObject {

    Q_OBJECT

public:
    Config(QObject *parent = nullptr);

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

    //
    // Get the portaudio device id to use for sound playback.
    //
    int deviceId() const noexcept;

    //
    // The sampling rate to use, the rate is a value in the audio::Samplerate enum
    //
    int samplerate() const noexcept;

    unsigned buffersize() const noexcept;

    unsigned volume() const noexcept;

    int gain(trackerboy::ChType ch) const noexcept;


    void setDeviceId(int device);

    void setSamplerate(int samplerate);

    void setBuffersize(unsigned buffersize);

    void setVolume(unsigned volume);

    void setGain(trackerboy::ChType ch, int gain);



signals:
    void deviceChanged(int deviceId);
    void samplerateChanged(int samplerate);
    void buffersizeChanged(unsigned buffersize);
    void volumeChanged(unsigned volume);
    void gainChanged(int channel, int value);

private:

    // device settings
    int mDeviceId;          // PortAudio device id for output
    int mSamplerate;      // Samplerate to use (see audio::Samplerate enum)
    unsigned mBuffersize;   // Buffer size of playback queue in milleseconds
    unsigned mVolume;       // Master volume of playback queue output, 0-100

    // mixer settings
    int mGains[4];     // Channel gain settings, in units of centibels (-120 cB to 120 cB or -12.0 dB to 12.0 dB)

};

