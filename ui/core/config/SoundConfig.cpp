
#include "core/config/SoundConfig.hpp"

#include "core/audio/AudioProber.hpp"
#include "core/config/keys.hpp"
#include "core/samplerates.hpp"

#include <QtDebug>

#define TU SoundConfigStatic
namespace TU {
    static auto const LOG_PREFIX = "[SoundConfig]";
}

SoundConfig::SoundConfig() :
    mBackendIndex(-1),
    mDeviceIndex(0),
    mSamplerateIndex(4),
    mLatency(40),
    mPeriod(5),
    mQuality(1)
{
}

int SoundConfig::backendIndex() const {
    return mBackendIndex;
}

int SoundConfig::deviceIndex() const {
    return mDeviceIndex;
}

int SoundConfig::samplerate() const {
    return SAMPLERATE_TABLE[mSamplerateIndex];
}

int SoundConfig::samplerateIndex() const {
    return mSamplerateIndex;
}

int SoundConfig::latency() const {
    return mLatency;
}

int SoundConfig::period() const {
    return mPeriod;
}

int SoundConfig::quality() const {
    return mQuality;
}

void SoundConfig::setBackendIndex(int index) {
    if (index >= -1) {
        mBackendIndex = index;
    }
}

void SoundConfig::setDeviceIndex(int index) {
    if (index >= -1) {
        mDeviceIndex = index;
    }
}

void SoundConfig::setSamplerateIndex(int index) {
    if (index >= 0 && index < N_SAMPLERATES) {
        mSamplerateIndex = index;
    }
}

void SoundConfig::setSamplerate(int samplerate) {
    for (int i = 0; i < N_SAMPLERATES; ++i) {
        if ((int)SAMPLERATE_TABLE[i] == samplerate) {
            mSamplerateIndex = i;
            return;
        }
    }

    qWarning() << TU::LOG_PREFIX << "unknown samplerate";
}

void SoundConfig::setLatency(int latency) {
    if (latency < MIN_LATENCY || latency > MAX_LATENCY) {
        qWarning() << TU::LOG_PREFIX << "invalid latency";
        return;
    }

    mLatency = latency;

}

void SoundConfig::setPeriod(int period) {
    if (period < MIN_PERIOD || period > MAX_PERIOD) {
        qWarning() << TU::LOG_PREFIX << "invalid period";
        return;
    }
    mPeriod = period;
}

void SoundConfig::setQuality(int quality) {
    if (quality < MIN_QUALITY || quality > MAX_QUALITY) {
        qWarning() << TU::LOG_PREFIX << "invalid quality";
        return;
    }
    mQuality = quality;
}

void SoundConfig::readSettings(QSettings &settings) {
    settings.beginGroup(Keys::Sound);

    //
    // Sound configuration notes:
    // If the API cannot be found the first one available is chosen
    // If the device name cannot be found, the default one is chosen
    //

    auto &prober = AudioProber::instance();

    QString api = settings.value(Keys::api).toString();
    int backend;
    if (api.isEmpty()) {
        backend = 0; // default to first available
    } else {
        backend = prober.backendNames().indexOf(api);
        if (backend == -1) {
            qWarning().noquote() << TU::LOG_PREFIX << "audio API" << api << "not available";
            backend = 0; // default to the first one
        }
    }

    setBackendIndex(backend);
    prober.probe(backend);

    int deviceIndex = 0;
    QByteArray idData = settings.value(Keys::deviceId).toByteArray();
    if (idData.size() == sizeof(ma_device_id)) {
        deviceIndex = prober.indexOfDevice(
            backend,
            *reinterpret_cast<ma_device_id*>(idData.data())
        );

        if (deviceIndex == 0) {
            // device not available, use the default
            qWarning() << TU::LOG_PREFIX << "last configured device not available, using default";
        }
    }

    setDeviceIndex(deviceIndex);

    setSamplerate(settings.value(Keys::samplerate, samplerate()).toInt());
    setLatency(settings.value(Keys::latency, mLatency).toInt());
    setPeriod(settings.value(Keys::period, mPeriod).toInt());
    setQuality(settings.value(Keys::quality, mQuality).toInt());

    settings.endGroup();
}

void SoundConfig::writeSettings(QSettings &settings) const {
    settings.beginGroup(Keys::Sound);
    settings.remove(QString());

    QString api;
    auto &prober = AudioProber::instance();
    if (mBackendIndex != -1) {
        api = prober.backendNames()[mBackendIndex];
    }
    settings.setValue(Keys::api, api);

    QByteArray id;
    auto devId = prober.deviceId(mBackendIndex, mDeviceIndex);
    if (devId) {
        id = QByteArray(reinterpret_cast<char*>(devId), sizeof(ma_device_id));
    }
    settings.setValue(Keys::deviceId, id);

    settings.setValue(Keys::samplerate, samplerate());
    settings.setValue(Keys::latency, mLatency);
    settings.setValue(Keys::period, mPeriod);
    settings.setValue(Keys::quality, mQuality);

    settings.endGroup();
}

#undef TU
