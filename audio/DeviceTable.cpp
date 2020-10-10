
#include "audio.hpp"


namespace audio {


DeviceTable::DeviceTable() :
    mHostList(),
    mDeviceList()
{
    // query all devices
    int hostCount = Pa_GetHostApiCount();
    int deviceOffset = 0;
    for (int hostIndex = 0; hostIndex < hostCount; ++hostIndex) {

        auto hostInfo = Pa_GetHostApiInfo(hostIndex);
        int deviceCount = 0;
        int deviceDefault = 0;

        for (int i = 0; i != hostInfo->deviceCount; ++i) {

            // i is the device index in the current host api
            // get the actual device index
            int deviceId = Pa_HostApiDeviceIndexToDeviceIndex(hostIndex, i);
            // get the info
            auto info = Pa_GetDeviceInfo(deviceId);
            if (info->maxOutputChannels == 0) {
                // no output channels, probably a microphone. ignore it
                continue;
            }

            // output parameters to test the device with
            PaStreamParameters outputParam;

            outputParam.channelCount = 2;
            outputParam.device = deviceId;
            outputParam.hostApiSpecificStreamInfo = NULL;
            outputParam.suggestedLatency = info->defaultLowOutputLatency;
            outputParam.sampleFormat = paInt16;

            int samplerateFlags = 0;
            for (int rate = 0; rate != SR_COUNT; ++rate) {
                // now see if the device supports each samplerate
                double samplerate = SAMPLERATE_TABLE[rate];
                if (Pa_IsFormatSupported(nullptr, &outputParam, samplerate) == paFormatIsSupported) {
                    /// samplerate is supported, add it to the flags
                    samplerateFlags |= (1 << rate);
                }
            }

            // do not add the device if it doesn't support any of our samplerates
            if (samplerateFlags) {
                if (deviceId == hostInfo->defaultOutputDevice) {
                    deviceDefault = mDeviceList.size() - deviceOffset;
                }

                mDeviceList.push_back({ deviceId, samplerateFlags, info });
                ++deviceCount;
            }


        }

        if (deviceCount) {
            mHostList.push_back({ hostIndex, deviceOffset, deviceCount, deviceDefault, hostInfo });
            deviceOffset += deviceCount;
        }

    }
}

DeviceTable& DeviceTable::instance() {
    static DeviceTable table;
    return table;
}

bool DeviceTable::isEmpty() const noexcept {
    return mHostList.size() == 0;
}

DeviceTable::HostVec const& DeviceTable::hosts() const noexcept {
    return mHostList;
}

DeviceTable::HostIterator DeviceTable::hostsBegin() const noexcept {
    return mHostList.cbegin();
}

DeviceTable::HostIterator DeviceTable::hostsEnd() const noexcept {
    return mHostList.cend();
}

DeviceTable::DeviceIterator DeviceTable::devicesBegin(int host) const noexcept {
    return mDeviceList.cbegin() + mHostList[host].deviceOffset;
}

DeviceTable::DeviceIterator DeviceTable::devicesEnd(int host) const noexcept {
    auto &h = mHostList[host];
    return mDeviceList.cbegin() + h.deviceOffset + h.deviceCount;
}




}
