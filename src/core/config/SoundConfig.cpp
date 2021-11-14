
#include "core/config/SoundConfig.hpp"

#include "audio/AudioEnumerator.hpp"
#include "core/config/keys.hpp"
#include "core/StandardRates.hpp"

#include <QSettings>
#include <QtDebug>

#define TU SoundConfigTU
namespace TU {

    static auto const LOG_PREFIX = "[SoundConfig]";

}

SoundConfig::SoundConfig() :
    mBackendIndex(-1),
    mDeviceIndex(0),
    mSamplerateIndex(4),
    mLatency(40),
    mPeriod(5)
{
}

int SoundConfig::backendIndex() const {
    return mBackendIndex;
}

int SoundConfig::deviceIndex() const {
    return mDeviceIndex;
}

int SoundConfig::samplerate() const {
    return StandardRates::get(mSamplerateIndex);
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
    if (index >= 0 && index < (int)StandardRates::COUNT) {
        mSamplerateIndex = index;
    }
}

void SoundConfig::setSamplerate(int samplerate) {
    for (int i = 0; i < (int)StandardRates::COUNT; ++i) {
        if (StandardRates::get(i) == samplerate) {
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

void SoundConfig::readSettings(QSettings &settings, AudioEnumerator &enumerator) {
    settings.beginGroup(Keys::Sound);

    //
    // Sound configuration notes:
    // If the API cannot be found the first one available is chosen
    // If the device name cannot be found, the default one is chosen
    //

    QString api = settings.value(Keys::api).toString();
    int backend;
    if (api.isEmpty()) {
        backend = 0; // default to first available
    } else {
        backend = enumerator.backendNames().indexOf(api);
        if (backend == -1) {
            qWarning().noquote() << TU::LOG_PREFIX << "audio API" << api << "not available";
            backend = 0; // default to the first one
        }
    }

    setBackendIndex(backend);
    enumerator.populate(backend);

    auto deviceId = settings.value(Keys::deviceId);
    int device = enumerator.deserializeDevice(backend, deviceId);
    if (device == -1) {
        qWarning() << TU::LOG_PREFIX << "last configured device not available, using default";
        device = 0;
    }
    setDeviceIndex(device);

    setSamplerate(settings.value(Keys::samplerate, samplerate()).toInt());
    setLatency(settings.value(Keys::latency, mLatency).toInt());
    setPeriod(settings.value(Keys::period, mPeriod).toInt());

    settings.endGroup();
}

void SoundConfig::writeSettings(QSettings &settings, AudioEnumerator const& enumerator) const {
    settings.beginGroup(Keys::Sound);
    settings.remove(QString());

    QString api;
    // backend index is -1 when there are no available backends
    // which shouldn't be possible since the ma_backend_null is always enabled
    if (mBackendIndex != -1) {
        api = enumerator.backendNames().at(mBackendIndex);
    }

    settings.setValue(Keys::api, api);
    settings.setValue(Keys::deviceId, enumerator.serializeDevice(mBackendIndex, mDeviceIndex));
    settings.setValue(Keys::samplerate, samplerate());
    settings.setValue(Keys::latency, mLatency);
    settings.setValue(Keys::period, mPeriod);

    settings.endGroup();
}

#undef TU
