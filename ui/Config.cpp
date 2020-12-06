
#include "Config.hpp"
#include "samplerates.hpp"

#include <QSettings>


namespace {

constexpr int DEFAULT_SAMPLERATE_INDEX = 4; // 44100 Hz
constexpr unsigned DEFAULT_BUFFERSIZE = 40;
constexpr unsigned DEFAULT_VOLUME = 100;


}


Config::Config(Miniaudio &miniaudio) :
    mMiniaudio(miniaudio)
{
}

Config::Sound const& Config::sound() {
    return mSound;
}

void Config::readSettings() {
    QSettings settings;


    settings.beginGroup("config");
    
    QByteArray id = settings.value("deviceId").toByteArray();
    if (id.size() == sizeof(ma_device_id)) {

        int index = mMiniaudio.lookupDevice(reinterpret_cast<ma_device_id*>(id.data()));
        setDevice(index + 1);

        // if we don't find the device we will go back to the default device
        // TODO: let the user know via messagebox if this occurs
    } else {
        setDevice(0);
    }

    
    mSound.context = mMiniaudio.context();

    setSamplerate(settings.value("samplerateIndex", DEFAULT_SAMPLERATE_INDEX).toUInt());
    mSound.buffersize = settings.value("buffersize", DEFAULT_BUFFERSIZE).toUInt();
    mSound.volume = settings.value("volume", DEFAULT_VOLUME).toInt();
    mSound.lowLatency = settings.value("lowLatency", true).toBool();
}


void Config::writeSettings() {
    QSettings settings;
    settings.beginGroup("config");

    QByteArray barray;
    if (mSound.deviceIndex != 0) {
        ma_device_id *id = mMiniaudio.deviceId(mSound.deviceIndex - 1);
        barray = QByteArray(reinterpret_cast<char*>(id), sizeof(ma_device_id));
    }
    settings.setValue("deviceId", barray);

    settings.setValue("samplerateIndex", mSound.samplerateIndex);
    settings.setValue("buffersize", mSound.buffersize);
    settings.setValue("volume", mSound.volume);
    settings.setValue("lowLatency", mSound.lowLatency);
}

void Config::setDevice(int index) {
    mSound.deviceIndex = index;
    if (index == 0) {
        mSound.device = nullptr;
    } else {
        mSound.device = mMiniaudio.deviceId(index - 1);
    }
}

void Config::setSamplerate(int index) {
    mSound.samplerate = SAMPLERATE_TABLE[index];
    mSound.samplerateIndex = index;
}
