
#include "audio.hpp"


namespace audio {



DeviceManager::DeviceManager() :
    mTable(DeviceTable::instance()),
    mCurrentApi(0),
    mCurrentDevice(0),
    mSamplerateIndex(0),
    mCurrentSamplerate(SR_44100),
    mSamplerates(),
    mCurrentApiIter(mTable.hostsBegin()),
    mCurrentDeviceIter(mTable.devicesBegin(0))
{
    setCurrentApi(Pa_GetDefaultHostApi());
}

int DeviceManager::currentHost() const noexcept {
    return mCurrentApi;
}

int DeviceManager::currentDevice() const noexcept {
    return mCurrentDevice;
}

int DeviceManager::currentSamplerate() const noexcept {
    return mSamplerateIndex;
}

int DeviceManager::portaudioDevice() const noexcept {
    auto iter = mTable.devicesBegin(mCurrentApi) + mCurrentDevice;
    return iter->deviceId;
}

std::vector<Samplerate> const& DeviceManager::samplerates() const noexcept {
    return mSamplerates;
}

void DeviceManager::setCurrentApi(int index) {
    mCurrentApi = index;
    mCurrentApiIter = mTable.hostsBegin() + index;
    setCurrentDevice(mCurrentApiIter->deviceDefault);
}

void DeviceManager::setCurrentDevice(int index) {
    if (mCurrentDevice != index) {
        mCurrentDevice = index;
        mCurrentDeviceIter = mTable.devicesBegin(mCurrentApi) + index;
        // populate the samplerateVec with all available samplerates
        mSamplerates.clear();
        mSamplerateIndex = 0;
        for (int i = 0; i != SR_COUNT; ++i) {
            int samplerateFlag = 1 << i;
            if (!!(mCurrentDeviceIter->samplerates & samplerateFlag)) {
                Samplerate rate = static_cast<Samplerate>(i);
                if (mCurrentSamplerate == rate) {
                    mSamplerateIndex = mSamplerates.size();
                }
                mSamplerates.push_back(rate);
            }
        }
    }
}

void DeviceManager::setCurrentSamplerate(int index) {
    mSamplerateIndex = index;
    mCurrentSamplerate = mSamplerates[index];
}

void DeviceManager::setPortaudioDevice(int device) {
    auto info = Pa_GetDeviceInfo(device);
    int hostIndex = 0;
    for (auto &host : mTable.hosts()) {
        if (host.hostId == info->hostApi) {
            break;
        }
        ++hostIndex;
    }

    mCurrentApi = hostIndex;
    mCurrentApiIter = mTable.hostsBegin() + hostIndex;

    // find the table device index
    int devIndex = 0;
    auto devicesBegin = mTable.devicesBegin(hostIndex);
    auto devicesEnd = mTable.devicesEnd(hostIndex);
    for (auto iter = devicesBegin; iter != devicesEnd; ++iter) {
        if (iter->deviceId == device) {
            break;
        }
        ++devIndex;
    }

    setCurrentDevice(devIndex);
    
}



}
