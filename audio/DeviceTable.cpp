
#include "audio.hpp"


namespace audio {

const unsigned SAMPLERATE_TABLE[] = {
    11025,
    22050,
    44100,
    48000,
    96000
};

DeviceTable::DeviceTable() :
    mDeviceList(),
    mDefaultDeviceIndex(0)
{
}

DeviceTable::Device& DeviceTable::operator[](unsigned int index) {
    return mDeviceList[index];
}


bool DeviceTable::isEmpty() const noexcept {
    return mDeviceList.size() == 0;
}

int DeviceTable::defaultDevice() const noexcept {
    return mDefaultDeviceIndex;
}

DeviceTable::DeviceIterator DeviceTable::begin() const noexcept {
    return mDeviceList.cbegin();
}

DeviceTable::DeviceIterator DeviceTable::end() const noexcept {
    return mDeviceList.cend();
}

void DeviceTable::rescan(struct SoundIo *soundio) {
    soundio_flush_events(soundio);

    mDeviceList.clear();

    int count = soundio_output_device_count(soundio);
    int defaultIndex = soundio_default_output_device_index(soundio);
    mDefaultDeviceIndex = 0; // reset default

    for (int i = 0; i < count; ++i) {
        auto device = soundio_get_output_device(soundio, i);
        // check if device supports signed 16-bit native endian
        if (soundio_device_supports_format(device, SoundIoFormatS16NE) && !device->is_raw) {
            // now check sample rates
            int samplerateFlags = 0;
            for (int j = 0; j != SR_COUNT; ++j) {
                unsigned rate = SAMPLERATE_TABLE[j];
                if (soundio_device_supports_sample_rate(device, rate)) {
                    samplerateFlags |= 1 << j;
                }
            }

            // if the flags is 0, none of our standard rates are supported by
            // the device. Only add to the table if there is at least one
            if (samplerateFlags) {
                mDeviceList.push_back({ i, samplerateFlags });
                if (i == defaultIndex) {
                    mDefaultDeviceIndex = i;
                }
            }

        }

        soundio_device_unref(device);
    }
}




}
