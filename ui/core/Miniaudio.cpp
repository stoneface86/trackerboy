
#include "core/Miniaudio.hpp"

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
    auto result = ma_context_init(nullptr, 0, nullptr, &mContext);
    mInitialized = result == MA_SUCCESS;
    if (mInitialized) {
        ma_context_get_devices(&mContext, &mDeviceList, &mDeviceCount, nullptr, nullptr);
    }
    return result;
}

ma_context* Miniaudio::context() {
    if (mInitialized) {
        return &mContext;
    } else {
        return nullptr;
    }
}

QStringList Miniaudio::deviceNames() {
    QStringList list;
    if (mDeviceList != nullptr) {
        for (unsigned i = 0; i != mDeviceCount; ++i) {
            list.append(QString::fromLatin1(mDeviceList[i].name));
        }
    }
    return list;
}

ma_device_id* Miniaudio::deviceId(int index) {
    if (mDeviceList != nullptr) {
        return &mDeviceList[index].id;
    } else {
        return nullptr;
    }
}

int Miniaudio::lookupDevice(ma_device_id *id) {
    if (id == nullptr) {
        return -1;
    }

    for (unsigned i = 0; i != mDeviceCount; ++i) {
        if (memcmp(id, &mDeviceList[i].id, sizeof(ma_device_id)) == 0) {
            return i;
        }
    }
    return -1;
}

QString Miniaudio::deviceName(int index) {
    if (mDeviceList == nullptr) {
        return {};
    }

    return QString::fromLatin1(mDeviceList[index].name);
}

QString Miniaudio::backendName() {
    return QString::fromLatin1(ma_get_backend_name(mContext.backend));
}

QString Miniaudio::deviceIdString(int index) {
    if (mDeviceList == nullptr) {
        return {};
    }

    auto id = deviceId(index);

    switch (mContext.backend) {
        case ma_backend_wasapi:
            // wchar_t string
            return QString::fromWCharArray(id->wasapi);
        case ma_backend_dsound:
            // GUID
            return {};
        case ma_backend_winmm:
            // uint
            return QString::number(id->winmm);
        case ma_backend_coreaudio:
            return QString::fromLatin1(id->coreaudio);
        case ma_backend_sndio:
            return QString::fromLatin1(id->sndio);
        case ma_backend_audio4:
            return QString::fromLatin1(id->audio4);
        case ma_backend_oss:
            return QString::fromLatin1(id->oss);
        case ma_backend_pulseaudio:
            return QString::fromLatin1(id->pulse);
        case ma_backend_alsa:
            return QString::fromLatin1(id->alsa);
        case ma_backend_aaudio:
            return QString::number(id->aaudio);
        case ma_backend_opensl:
            return QString::number(id->opensl);
        case ma_backend_null:
            return QString::number(id->nullbackend);
        case ma_backend_webaudio:
        case ma_backend_jack:
        default:
            return QStringLiteral("N/A");
    }
}

ma_device_info const* Miniaudio::deviceInfo(int index) {
    if (mDeviceList == nullptr) {
        return nullptr;
    }

    return mDeviceList + index;
}
