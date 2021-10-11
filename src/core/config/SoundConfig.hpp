
#pragma once

#include <QSettings>


class SoundConfig {

public:
    static constexpr int MIN_PERIOD = 1;
    static constexpr int MAX_PERIOD = 100;

    static constexpr int MIN_LATENCY = 1;
    static constexpr int MAX_LATENCY = 500;

    SoundConfig();
    
    int backendIndex() const;
    int deviceIndex() const;
    int samplerate() const;
    int samplerateIndex() const;
    int latency() const;
    int period() const;

    void setBackendIndex(int index);

    void setDeviceIndex(int index);

    void setSamplerateIndex(int index);

    void setSamplerate(int samplerate);

    void setLatency(int latency);

    void setPeriod(int period);
    
    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    int mBackendIndex;           // backend index in AudioProber list (-1 for no backend)
    int mDeviceIndex;            // device index from AudioProber device list
    int mSamplerateIndex;        // index of the current samplerate
    int mLatency;                // latency, or internal buffer size, in milliseconds
    int mPeriod;                 // period, in milliseconds
};
