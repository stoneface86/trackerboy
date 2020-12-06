
#include "Miniaudio.hpp"


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
