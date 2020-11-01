
#include "Config.hpp"
#include "samplerates.hpp"

namespace {

constexpr int DEFAULT_SAMPLERATE_INDEX = 4; // 44100 Hz
constexpr unsigned DEFAULT_BUFFERSIZE = 40;
constexpr unsigned DEFAULT_VOLUME = -3;


}


Config::Config()
{
    auto err = ma_context_init(nullptr, 0, nullptr, &mContext);
    assert(err == MA_SUCCESS);
    mSound.context = &mContext;
}

Config::~Config() {
    ma_context_uninit(&mContext);
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
    getDevices();
    setDevice(0);

    QByteArray id = settings.value("deviceId").toByteArray();
    if (id.size() == sizeof(ma_device_id)) {
        // search all devices for the id in the config
        for (unsigned i = 0; i != mDeviceCount; ++i) {
            if (memcmp(id.data(), &mDeviceList[i].id, sizeof(ma_device_id)) == 0) {
                // found it, index is offset by 1 (0 is the default device)
                setDevice(i + 1);
                break;
            }
        }

        // if we don't find the device we will go back to the default device
        // TODO: let the user know via messagebox if this occurs
    }

    


    setSamplerate(settings.value("samplerateIndex", DEFAULT_SAMPLERATE_INDEX).toUInt());
    mSound.buffersize = settings.value("buffersize", DEFAULT_BUFFERSIZE).toUInt();
    mSound.volume = settings.value("volume", DEFAULT_VOLUME).toInt();
    mSound.lowLatency = settings.value("lowLatency", true).toBool();

    applySound();
}


void Config::writeSettings(QSettings &settings) {
    settings.beginGroup("config");

    QByteArray barray;
    if (mSound.deviceIndex != 0) {
        ma_device_id *id = &mDeviceList[mSound.deviceIndex - 1].id;
        barray = QByteArray(reinterpret_cast<char*>(id), sizeof(ma_device_id));
    }
    settings.setValue("deviceId", barray);

    settings.setValue("samplerateIndex", mSound.samplerateIndex);
    settings.setValue("buffersize", mSound.buffersize);
    settings.setValue("volume", mSound.volume);
    settings.setValue("lowLatency", mSound.lowLatency);
}

void Config::applySound() {
    emit soundConfigChanged();
}

void Config::getDevices() {
    auto err = ma_context_get_devices(&mContext, &mDeviceList, &mDeviceCount, nullptr, nullptr);
    assert(err == MA_SUCCESS);
}


void Config::setDevice(int index) {
    mSound.deviceIndex = index;
    if (index == 0) {
        mSound.device = nullptr;
    } else {
        mSound.device = &mDeviceList[index - 1].id;
    }
}

void Config::setSamplerate(int index) {
    mSound.samplerate = SAMPLERATE_TABLE[index];
    mSound.samplerateIndex = index;
}
