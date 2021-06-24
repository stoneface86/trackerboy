
#include "core/Miniaudio.hpp"

#include <QCoreApplication>
#include <QMessageLogger>
#include <QDebug>

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


Miniaudio::Miniaudio() :
    mInitialized(false),
    mContext(),
    mDeviceList(nullptr),
    mDeviceCount(0)
{
}

Miniaudio::~Miniaudio() {
    if (mInitialized) {
        ma_context_uninit(&mContext);
    }
}

ma_result Miniaudio::init() {
    return initImpl(nullptr, 0);
}

ma_result Miniaudio::init(ma_backend const backend) {
    return initImpl(&backend, 1);
}

ma_result Miniaudio::initImpl(ma_backend const *backends, ma_uint32 count) {
    auto config = ma_context_config_init();
    config.logCallback = logCallback;
    auto result = ma_context_init(backends, count, &config, &mContext);
    mInitialized = result == MA_SUCCESS;
    rescan();

    return result;
}

ma_context* Miniaudio::context() {
    if (mInitialized) {
        return &mContext;
    } else {
        return nullptr;
    }
}

unsigned Miniaudio::deviceCount() const {
    return mDeviceCount;
}

QStringList Miniaudio::deviceNames() {
    QStringList list;
    if (mDeviceList != nullptr) {
        for (unsigned i = 0; i != mDeviceCount + 1; ++i) {
            list.append(deviceName(i));
        }
    }
    return list;
}

ma_device_id* Miniaudio::deviceId(int index) {
    if (mDeviceList != nullptr) {
        return index == 0 ? nullptr : &mDeviceList[index - 1].id;
    } else {
        return nullptr;
    }
}

int Miniaudio::lookupDevice(ma_device_id *id) {
    if (id == nullptr) {
        return 0;
    }

    for (unsigned i = 0; i != mDeviceCount; ++i) {
        if (memcmp(id, &mDeviceList[i].id, sizeof(ma_device_id)) == 0) {
            return i + 1;
        }
    }
    return -1;
}

QString Miniaudio::deviceName(int index) {
    if (mDeviceList == nullptr) {
        return {};
    }

    if (index == 0) {
        return QCoreApplication::tr("Default device");
    } else {
        return QString::fromUtf8(mDeviceList[index - 1].name);
    }
}

QString Miniaudio::backendName() {
    return QString::fromLatin1(ma_get_backend_name(mContext.backend));
}

QString Miniaudio::deviceIdString(ma_device_id const& id) {

    switch (mContext.backend) {
        case ma_backend_wasapi:
            // wchar_t string
            return QString::fromWCharArray(id.wasapi);
        case ma_backend_dsound:
            // GUID
            return {};
        case ma_backend_winmm:
            // uint
            return QString::number(id.winmm);
        case ma_backend_coreaudio:
            return QString::fromUtf8(id.coreaudio);
        case ma_backend_sndio:
            return QString::fromUtf8(id.sndio);
        case ma_backend_audio4:
            return QString::fromUtf8(id.audio4);
        case ma_backend_oss:
            return QString::fromUtf8(id.oss);
        case ma_backend_pulseaudio:
            return QString::fromUtf8(id.pulse);
        case ma_backend_alsa:
            return QString::fromUtf8(id.alsa);
        case ma_backend_aaudio:
            return QString::number(id.aaudio);
        case ma_backend_opensl:
            return QString::number(id.opensl);
        case ma_backend_null:
            return QString::number(id.nullbackend);
        case ma_backend_webaudio:
        case ma_backend_jack:
        default:
            return QCoreApplication::tr("N/A");
    }
}

void Miniaudio::rescan() {
    if (mInitialized) {
        ma_context_get_devices(&mContext, &mDeviceList, &mDeviceCount, nullptr, nullptr);
    }
}
