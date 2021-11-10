
#include "audio/AudioEnumerator.hpp"

#include <QCoreApplication>
#include <QtDebug>

#define TU AudioEnumeratorTU

namespace TU {
//
// logging callback for miniaudio, redirects to Qt's message logging utility
//
void logCallback(ma_context* pContext, ma_device* pDevice, ma_uint32 logLevel, const char* message) {
    Q_UNUSED(pContext)
    Q_UNUSED(pDevice)

    QMessageLogger logger;

    [logLevel, &logger]() {
        switch (logLevel) {
            case MA_LOG_LEVEL_ERROR:
                return logger.critical();
            case MA_LOG_LEVEL_WARNING:
                return logger.warning();
            default:
                return logger.info();
        }
    }() << "[Miniaudio]" << message;
}

}


AudioEnumerator::Context::Context(ma_backend backend) :
    mContext(),
    mInitialized(false),
    mBackend(backend),
    mDevices()
{
}

AudioEnumerator::Context::~Context() {
    if (mContext != nullptr && mInitialized) {
        ma_context_uninit(mContext.get());
    }
}

ma_backend AudioEnumerator::Context::backend() const {
    return mBackend;
}

//
// Gets the miniaudio context for this backend
//
ma_context* AudioEnumerator::Context::get() {
    if (mContext == nullptr) {
        // lazy loading
        mContext.reset(new ma_context);
        probe();
    }


    return mContext.get();
}

ma_context* AudioEnumerator::Context::get() const {
    return mContext.get();
}

ma_device_id const* AudioEnumerator::Context::id(int deviceIndex) const {
    if (deviceIndex <= 0 || deviceIndex > (int)mDevices.size()) {
        return nullptr; // default device
    }

    return &mDevices[deviceIndex - 1].id;
}

bool AudioEnumerator::Context::initialized() const {
    return mInitialized;
}

int AudioEnumerator::Context::findDevice(ma_device_id const& id) const {
    int i = 0;
    for (auto &info : mDevices) {
        if (memcmp(&id, &info.id, sizeof(ma_device_id)) == 0) {
            return (int)i + 1;
        }
        ++i;
    }

    return 0;
}

QStringList AudioEnumerator::Context::deviceNames() const {
    QStringList list;

    list.append(QCoreApplication::tr("Default device"));
    for (auto &info : mDevices) {
        list.append(QString::fromUtf8(info.name));
    }

    return list;
}

int AudioEnumerator::Context::devices() const {
    return (int)mDevices.size() + 1;
}

//
// Probe all devices in the miniaudio context, updating ids and names
//
void AudioEnumerator::Context::probe() {
    if (mContext == nullptr) {
        get(); // get calls probe when allocating context
        return;
    }

    if (!mInitialized) {
        // context not initialized, attempt to do so and log error on failure

        auto config = ma_context_config_init();
        config.logCallback = TU::logCallback;
        auto result = ma_context_init(&mBackend, 1, &config, mContext.get());
        if (result == MA_SUCCESS) {
            mInitialized = true;
        } else {
            qCritical().nospace() << "Failed to initialize audio backend '"
                                  << ma_get_backend_name(mBackend) << "': "
                                  << ma_result_description(result);
            // no context, nothing to probe
            return;
        }

    }

    mDevices.clear();
    auto result = ma_context_enumerate_devices(mContext.get(), enumerateCallback, this);

    if (result != MA_SUCCESS) {
        mDevices.clear();
    }
}


ma_bool32 AudioEnumerator::Context::enumerateCallback(
    ma_context* pContext,
    ma_device_type deviceType,
    const ma_device_info* pInfo,
    void* pUserData
) {
    Q_UNUSED(pContext)
    if (deviceType == ma_device_type_playback) {
        static_cast<Context*>(pUserData)->enumerate(pInfo);
    }

    return MA_TRUE;

}

void AudioEnumerator::Context::enumerate(ma_device_info const* info) {
    mDevices.push_back(*info);
}



AudioEnumerator::AudioEnumerator()
{
    std::array<ma_backend, MA_BACKEND_COUNT> backendArr;
    size_t count;
    ma_get_enabled_backends(backendArr.data(), backendArr.size(), &count);

    for (size_t i = 0; i < count; ++i) {
        auto const backend = backendArr[i];
        mContexts.emplace_back(backend);
        mBackendNames.append(QString::fromUtf8(ma_get_backend_name(backend)));
    }
}

bool AudioEnumerator::backendIsAvailable(int backend) const {
    if (indexIsInvalid(backend)) {
        return false;
    }

    return mContexts[backend].initialized();
}

QStringList AudioEnumerator::backendNames() const {
    return mBackendNames;
}

int AudioEnumerator::backends() const {
    return mBackendNames.count();
}

QStringList AudioEnumerator::deviceNames(int backend) const {
    if (indexIsInvalid(backend)) {
        return {};
    }

    return mContexts[backend].deviceNames();
}

int AudioEnumerator::devices(int backend) const {
    if (indexIsInvalid(backend)) {
        return 0;
    }

    return mContexts[backend].devices();
}

AudioEnumerator::Device AudioEnumerator::device(int backend, int device) const {
    if (indexIsInvalid(backend)) {
        return { nullptr, nullptr };
    } else {
        return {
            mContexts[backend].get(),
            mContexts[backend].id(device)
        };
    }
}

void AudioEnumerator::populate(int backend) {
    if (indexIsInvalid(backend)) {
        return;
    }

    mContexts[backend].probe();
}

QVariant AudioEnumerator::serializeDevice(int backend, int device) const {
    if (indexIsInvalid(backend)) {
        return {};
    }

    auto id = mContexts[backend].id(device);
    if (id == nullptr) {
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(id), sizeof(ma_device_id));;
}

int AudioEnumerator::deserializeDevice(int backend, const QVariant &data) const {
    if (indexIsInvalid(backend)) {
        return -1;
    }

    auto idData = data.toByteArray();
    if (idData.isEmpty()) {
        return 0; // default device
    } else if (idData.size() != sizeof(ma_device_id)) {
        return -1;
    }
    return mContexts[backend].findDevice(reinterpret_cast<ma_device_id const&>(*idData.data()));
}

bool AudioEnumerator::indexIsInvalid(int backendIndex) const {
    return backendIndex < 0 || backendIndex > (int)mContexts.size();
}



#undef TU
