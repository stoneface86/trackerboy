
#include "Config.hpp"

#include "audio.hpp"

namespace {

constexpr int DEFAULT_SAMPLERATE = audio::SR_44100;
constexpr unsigned DEFAULT_BUFFERSIZE = 40;
constexpr unsigned DEFAULT_VOLUME = 100;
constexpr int DEFAULT_GAIN = 0;


}


Config::Config(audio::BackendTable &backendTable, QObject *parent) :
    //mDeviceConfig(),
    mBackendTable(backendTable),
    mBackendIndex(-1),
    mDeviceIndex(-1),
    mSoundio(nullptr),
    mDevice(nullptr),
    mSamplerate(0),
    mBuffersize(0),
    mVolume(0),
    mGains{0},
    QObject(parent)
{
}

int Config::backendIndex() const noexcept {
    return mBackendIndex;
}

int Config::deviceIndex() const noexcept {
    return mDeviceIndex;
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

void Config::setDevice(int backend, int device) {
    bool getDevice = false;

    if (backend != mBackendIndex) {
        mBackendIndex = backend;
        mSoundio = mBackendTable[backend].soundio;
        getDevice = true;
    }

    if (device != mDeviceIndex) {
        mDeviceIndex = device;
        getDevice = true;
    }

    if (getDevice) {
        soundio_device_unref(mDevice);
        mDevice = mBackendTable.getDevice(audio::BackendTable::Location(backend, device));
        //emit deviceChanged();
    }
}

void Config::setSamplerate(unsigned samplerate) {
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
    // device config
    int backend = settings.value("deviceBackend", SoundIoBackendNone).toInt();
    QByteArray id = settings.value("deviceId", QByteArray('\0', 1)).toByteArray();
    audio::BackendTable::Location location;
    if (backend == SoundIoBackendNone) {
        // first time launching the application or user reset configuration
        // get the default device
        location = mBackendTable.getDefaultDeviceLocation();
    } else {
        // set the device from the id in the configuration, falling back to default
        // TODO: display a messagebox if we couldn't find the device (possibly disconnected)

        location = mBackendTable.getDeviceLocation(static_cast<SoundIoBackend>(backend), id.data());
    }
    mBackendIndex = location.first;
    mDeviceIndex = location.second;
    mDevice = mBackendTable.getDevice(location);
    mSoundio = mBackendTable[mBackendIndex].soundio;

    mSamplerate = settings.value("samplerate", DEFAULT_SAMPLERATE).toInt();
    mBuffersize = settings.value("buffersize", DEFAULT_BUFFERSIZE).toUInt();
    mVolume = settings.value("volume", DEFAULT_VOLUME).toUInt();
    for (int i = 0; i != 4; ++i) {
        mGains[i] = settings.value(QString("gain%1").arg(i + 1), DEFAULT_GAIN).toInt();
    }
}


void Config::writeSettings(QSettings &settings) {
    settings.beginGroup("config");
    settings.setValue("deviceBackend", static_cast<int>(mSoundio->current_backend));
    QByteArray id(mDevice->id);
    settings.setValue("deviceId", id);
    settings.setValue("samplerate", mSamplerate);
    settings.setValue("buffersize", mBuffersize);
    settings.setValue("volume", mVolume);
    for (int i = 0; i != 4; ++i) {
        settings.setValue(QString("gain%1").arg(i + 1), mGains[i]);
    }
}
