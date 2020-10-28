
#include "Config.hpp"
#include "samplerates.hpp"

#include "audio.hpp"

namespace {

constexpr int DEFAULT_SAMPLERATE_INDEX = 2; // 44100 Hz
constexpr unsigned DEFAULT_BUFFERSIZE = 40;
constexpr unsigned DEFAULT_VOLUME = 100;
constexpr unsigned DEFAULT_BASS_FREQUENCY = 20;
constexpr int DEFAULT_TREBLE = -8;
constexpr unsigned DEFAULT_TREBLE_FREQUENCY = 12000;


}


Config::Config()
{
}

Config::Sound const& Config::sound() {
    return mSound;
}

//void Config::setDevice(int backend, int device) {
//    bool getDevice = false;
//
//    if (backend != mSound.backendIndex) {
//        mSound.backendIndex = backend;
//        mSound.soundio = mBackendTable[backend].soundio;
//        getDevice = true;
//    }
//
//    if (device != mSound.deviceIndex) {
//        mSound.deviceIndex = device;
//        getDevice = true;
//    }
//
//    if (getDevice) {
//        soundio_device_unref(mSound.device);
//        mSound.device = mBackendTable.getDevice(audio::BackendTable::Location(backend, device));
//    }
//}

void Config::readSettings(QSettings &settings) {
    settings.beginGroup("config");
    // device config
    //int backend = settings.value("deviceBackend", SoundIoBackendNone).toInt();
    //QByteArray id = settings.value("deviceId").toByteArray();
    //audio::BackendTable::Location location;
    //if (backend == SoundIoBackendNone) {
    //    // first time launching the application or user reset configuration
    //    // get the default device
    //    location = mBackendTable.getDefaultDeviceLocation();
    //} else {
    //    // set the device from the id in the configuration, falling back to default
    //    // TODO: display a messagebox if we couldn't find the device (possibly disconnected)

    //    location = mBackendTable.getDeviceLocation(static_cast<SoundIoBackend>(backend), id.data());
    //}
    //mSound.backendIndex = location.first;
    //mSound.deviceIndex = location.second;
    //mSound.device = mBackendTable.getDevice(location);
    //mSound.soundio = mBackendTable[mSound.backendIndex].soundio;

    mSound.samplerateIndex = settings.value("samplerateIndex", DEFAULT_SAMPLERATE_INDEX).toUInt();
    mSound.samplerate = SAMPLERATE_TABLE[mSound.samplerateIndex];
    mSound.buffersize = settings.value("buffersize", DEFAULT_BUFFERSIZE).toUInt();
    mSound.volume = settings.value("volume", DEFAULT_VOLUME).toUInt();
    mSound.bassFrequency = settings.value("bassFrequency", DEFAULT_BASS_FREQUENCY).toUInt();
    mSound.treble = settings.value("treble", DEFAULT_TREBLE).toInt();
    mSound.trebleFrequency = settings.value("trebleFrequency", DEFAULT_TREBLE_FREQUENCY).toUInt();

    applySound();
}


void Config::writeSettings(QSettings &settings) {
    settings.beginGroup("config");
    //settings.setValue("deviceBackend", static_cast<int>(mSound.soundio->current_backend));
    //QByteArray id(mSound.device->id);
    //settings.setValue("deviceId", id);
    settings.setValue("samplerateIndex", mSound.samplerateIndex);
    settings.setValue("buffersize", mSound.buffersize);
    settings.setValue("volume", mSound.volume);
    settings.setValue("bassFrequency", mSound.bassFrequency);
    settings.setValue("treble", mSound.treble);
    settings.setValue("trebleFrequency", mSound.trebleFrequency);
}

void Config::applySound() {
    emit soundConfigChanged();
}
