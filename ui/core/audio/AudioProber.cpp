
#include "core/audio/AudioProber.hpp"

#include <array>

#include <QMutexLocker>
#include <QtDebug>


//
// logging callback for miniaudio, redirects to Qt's message logging utility
//
void logCallback(ma_context* pContext, ma_device* pDevice, ma_uint32 logLevel, const char* message) {
    Q_UNUSED(pContext)
    Q_UNUSED(pDevice)

    static QMessageLogger logger;
    
    QDebug dbg = (logLevel == MA_LOG_LEVEL_ERROR) ? logger.critical() :
        ((logLevel == MA_LOG_LEVEL_WARNING) ? logger.warning() : logger.info());
    dbg << "[Miniaudio]" << message;
}


AudioProber::Context::Context(ma_backend backend) :
    mContext(),
    mBackend(backend),
    mDevices()
{
}

AudioProber::Context::~Context() {
    if (mContext != nullptr) {
        ma_context_uninit(mContext.get());
    }
}

ma_backend AudioProber::Context::backend() const {
    return mBackend;
}

ma_context* AudioProber::Context::get() {
    if (mContext == nullptr) {
        mContext.reset(new ma_context);

        auto config = ma_context_config_init();
        config.logCallback = logCallback;
        ma_context_init(&mBackend, 1, &config, mContext.get());
        probe();
    }
    return mContext.get();
}

ma_device_id* AudioProber::Context::id(int deviceIndex) {
    if (deviceIndex <= 0 || deviceIndex > (int)mDevices.size()) {
        return nullptr; // default device
    }

    return &mDevices[deviceIndex - 1].id;
}

int AudioProber::Context::findDevice(ma_device_id const& id) const {
    int i = 0;
    for (auto &info : mDevices) {
        if (memcmp(&id, &info.id, sizeof(ma_device_id)) == 0) {
            return (int)i + 1;
        }
        ++i;
    }

    return 0;
}

QStringList AudioProber::Context::deviceNames() const {
    QStringList list;

    for (auto &info : mDevices) {
        list.append(QString::fromUtf8(info.name));
    }

    return list;
}


ma_bool32 AudioProber::Context::enumerateCallback(
    ma_context* pContext,
    ma_device_type deviceType,
    const ma_device_info* pInfo,
    void* pUserData
) {
    Q_UNUSED(pContext)
    if (deviceType == ma_device_type_playback) {
        static_cast<AudioProber::Context*>(pUserData)->enumerate(pInfo);
    }

    return MA_TRUE;

}


void AudioProber::Context::enumerate(const ma_device_info* info) {
    mDevices.push_back(*info);
}


void AudioProber::Context::probe() {
    if (mContext == nullptr) {
        get(); // get calls probe when allocating context
        return;
    }

    mDevices.clear();
    auto result = ma_context_enumerate_devices(mContext.get(), enumerateCallback, this);

    if (result != MA_SUCCESS) {
        mDevices.clear();
    }
}


AudioProber::AudioProber() :
    mContexts()
{

    std::array<ma_backend, MA_BACKEND_COUNT> backendArr;
    size_t count;
    ma_get_enabled_backends(backendArr.data(), backendArr.size(), &count);

    for (size_t i = 0; i < count; ++i) {
        mContexts.emplace_back(backendArr[i]);
    }

}


int AudioProber::indexOfBackend(ma_backend backend) const {

    int index = 0;
    for (auto const& context : mContexts) {
        if (context.backend() == backend) {
            return index;
        }
        ++index;
    }
    return -1;

}

int AudioProber::indexOfDevice(int backendIndex, ma_device_id const& id) const {

    if (indexIsInvalid(backendIndex)) {
        return -1;
    }

    return mContexts[backendIndex].findDevice(id);
}

ma_context* AudioProber::context(int backendIndex) {
    if (indexIsInvalid(backendIndex)) {
        return nullptr;
    }

    return mContexts[backendIndex].get();
}

ma_device_id* AudioProber::deviceId(int backendIndex, int deviceIndex) {
    if (indexIsInvalid(backendIndex)) {
        return nullptr;
    }

    return mContexts[backendIndex].id(deviceIndex);
}

QStringList AudioProber::backendNames() const {

    QStringList list;
    for (auto const& context : mContexts) {
        auto name = ma_get_backend_name(context.backend());
        list.append(QString::fromUtf8(name));
    }
    return list;
}

QStringList AudioProber::deviceNames(int backendIndex) const {
    if (indexIsInvalid(backendIndex)) {
        return {};
    }

    return mContexts[backendIndex].deviceNames();
}

AudioProber& AudioProber::instance() {
    static AudioProber prober;
    return prober;
}

void AudioProber::probe(int backendIndex) {
    if (indexIsInvalid(backendIndex)) {
        return;
    }

    mContexts[backendIndex].probe();
}

bool AudioProber::indexIsInvalid(int backendIndex) const {
    return backendIndex < 0 || backendIndex > (int)mContexts.size();
}
