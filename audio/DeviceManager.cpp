
#include "audio.hpp"

namespace {

const double SAMPLERATE_TABLE[] = {
    11025.0,
    22050.0,
    44100.0,
    48000.0,
    96000.0
};

}


namespace audio {


DeviceManager::DeviceManager() :
    mCurrentApi(0),
    mCurrentDevice(0),
    mCurrentSamplerate(Samplerate::s44100),
    mApis(),
    mDeviceList()
{
    // query all available APIs

    int apiCount = Pa_GetHostApiCount();
    for (int i = 0; i < apiCount; ++i) {
        mApis.push_back(Pa_GetHostApiInfo(i));
    }

    setCurrentApi(Pa_GetDefaultHostApi());
}

std::vector<DeviceManager::Device> const& DeviceManager::devices() const noexcept {
    return mDeviceList;
}

std::vector<PaHostApiInfo const *> const& DeviceManager::hosts() const noexcept {
    return mApis;
}

int DeviceManager::currentHost() const noexcept {
    return mCurrentApi;
}

int DeviceManager::currentDevice() const noexcept {
    return mCurrentApi;
}

void DeviceManager::getOutputParameters(PaStreamParameters &param) {
    auto &device = mDeviceList[mCurrentDevice];
    param.channelCount = 2;
    param.device = mCurrentDevice;
    param.hostApiSpecificStreamInfo = NULL;
    param.suggestedLatency = device.mInfo->defaultLowOutputLatency;
    param.sampleFormat = paFloat32;
}

void DeviceManager::queryDevices() {
    mDeviceList.clear();

    auto hostInfo = mApis[mCurrentApi];
    mCurrentDevice = 0;

    for (int i = 0; i != hostInfo->deviceCount; ++i) {

        int deviceId = Pa_HostApiDeviceIndexToDeviceIndex(mCurrentApi, i);
        auto info = Pa_GetDeviceInfo(deviceId);
        if (info->maxOutputChannels == 0) {
            // no output channels, probably a microphone
            // or is not from the current api
            continue;
        }

        PaStreamParameters outputParam;

        outputParam.channelCount = 2;
        outputParam.device = deviceId;
        outputParam.hostApiSpecificStreamInfo = NULL;
        outputParam.suggestedLatency = info->defaultLowOutputLatency;
        outputParam.sampleFormat = paFloat32;

        int sampleRateFlags = 0;
        for (int j = 0; j != 5; ++j) {
            // now see if the device supports each samplerate
            double samplerate = SAMPLERATE_TABLE[j];
            if (Pa_IsFormatSupported(nullptr, &outputParam, samplerate) == paFormatIsSupported) {
                /// samplerate is supported, add it to the flags
                sampleRateFlags |= (1 << j);
            }
        }
        
        // do not add the device if it doesn't support any of our samplerates
        if (sampleRateFlags) {
            mDeviceList.push_back({ deviceId, sampleRateFlags, info });
            if (deviceId == hostInfo->defaultOutputDevice) {
                mCurrentDevice = i;
            }
        }
        

    }
}

void DeviceManager::setCurrentApi(int index) {
    mCurrentApi = index;
    queryDevices();
}



}
