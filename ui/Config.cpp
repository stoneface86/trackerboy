
#include "Config.hpp"

#include "portaudio.h"
#include "audio.hpp"

namespace {

constexpr int DEFAULT_SAMPLERATE = audio::SR_44100;
constexpr unsigned DEFAULT_BUFFERSIZE = 40;
constexpr unsigned DEFAULT_VOLUME = 100;
constexpr int DEFAULT_GAIN = 0;


}


Config::Config(QObject *parent) :
    mDeviceId(0),
    mSamplerate(0),
    mBuffersize(0),
    mVolume(0),
    mGains{0},
    QObject(parent)
{
}

int Config::deviceId() const noexcept {
    return mDeviceId;
}

int Config::samplerate() const noexcept {
    return mSamplerate;
}

unsigned Config::buffersize() const noexcept {
    return mBuffersize;
}

unsigned Config::volume() const noexcept {
    return mVolume;
}

int Config::gain(trackerboy::ChType ch) const noexcept {
    return mGains[static_cast<int>(ch)];
}

void Config::setDeviceId(int device) {
    if (mDeviceId != device) {
        mDeviceId = device;
        emit deviceChanged(device);
    }
}

void Config::setSamplerate(int samplerate) {
    if (mSamplerate != samplerate) {
        mSamplerate = samplerate;
        emit samplerateChanged(samplerate);
    }
}

void Config::setBuffersize(unsigned buffersize) {
    if (mBuffersize != buffersize) {
        mBuffersize = buffersize;
        emit buffersizeChanged(buffersize);
    }
}

void Config::setVolume(unsigned volume) {
    if (mVolume != volume) {
        mVolume = volume;
        emit volumeChanged(volume);
    }
}

void Config::setGain(trackerboy::ChType ch, int gain) {
    auto &gainVar = mGains[static_cast<int>(ch)];
    if (gainVar != gain) {
        gainVar = gain;
        emit gainChanged(static_cast<int>(ch), gain);
    }
}


void Config::readSettings(QSettings &settings) {
    settings.beginGroup("config");
    mDeviceId = settings.value("deviceId", Pa_GetDefaultOutputDevice()).toInt();
    mSamplerate = settings.value("samplerate", DEFAULT_SAMPLERATE).toInt();
    mBuffersize = settings.value("buffersize", DEFAULT_BUFFERSIZE).toUInt();
    mVolume = settings.value("volume", DEFAULT_VOLUME).toUInt();
    for (int i = 0; i != 4; ++i) {
        mGains[i] = settings.value(QString("gain%1").arg(i + 1), DEFAULT_GAIN).toInt();
    }
}


void Config::writeSettings(QSettings &settings) {
    settings.beginGroup("config");
    settings.setValue("deviceId", mDeviceId);
    settings.setValue("samplerate", mSamplerate);
    settings.setValue("buffersize", mBuffersize);
    settings.setValue("volume", mVolume);
    for (int i = 0; i != 4; ++i) {
        settings.setValue(QString("gain%1").arg(i + 1), mGains[i]);
    }
}
