
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

public:
    Config(audio::BackendTable &backendTable, QObject *parent = nullptr);

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

    int backendIndex() const noexcept;

    int deviceIndex() const noexcept;

    //
    // The sampling rate to use, the rate is a value in the audio::Samplerate enum
    //
    int samplerate() const noexcept;

    unsigned buffersize() const noexcept;

    unsigned volume() const noexcept;

    int gain(trackerboy::ChType ch) const noexcept;

    void setDevice(int backend, int device);

    void setSamplerate(unsigned samplerate);

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
    audio::BackendTable &mBackendTable;

    // device settings
    int mBackendIndex;
    int mDeviceIndex;
    struct SoundIo *mSoundio;
    struct SoundIoDevice *mDevice;
    unsigned mSamplerate;

    unsigned mBuffersize;   // Buffer size of playback queue in milleseconds
    unsigned mVolume;       // Master volume of playback queue output, 0-100

    // mixer settings
    int mGains[4];     // Channel gain settings, in units of centibels (-120 cB to 120 cB or -12.0 dB to 12.0 dB)

};

