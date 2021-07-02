
#include "core/audio/AudioProber.hpp"

#include <QMutexLocker>

AudioProber::Backend::Backend(RtAudio::Api api) :
    rtaudio(),
    api(api),
    deviceIds(),
    deviceNames(),
    defaultDevice(0)
{
}

Guarded<RtAudio>* AudioProber::Backend::get() {
    if (rtaudio == nullptr) {
        rtaudio.reset(new Guarded<RtAudio>(api));
        #ifndef QT_NO_DEBUG
        {
            auto handle = rtaudio->access(); 
            handle->showWarnings(true);
        }
        #endif
        probe();
    }
    return rtaudio.get();
}

void AudioProber::Backend::probe() {
    if (rtaudio == nullptr) {
        get(); // get calls probe when allocating rtaudio
        return;
    }

    defaultDevice = 0;
    deviceIds.clear();
    deviceNames.clear();

    auto handle = rtaudio->access();
    auto const count = handle->getDeviceCount();
    for (unsigned i = 0; i < count; ++i) {
        // filter all devices to ones we can use
        // 1. stereo output device (number of output channels >= 2)
        auto info = handle->getDeviceInfo(i);
        if (info.probed) { // if this is false an error has occurred within RtAudio
            if (info.outputChannels >= 2) {
                deviceIds.push_back(i);
                deviceNames.append(QString::fromStdString(info.name));
                if (info.isDefaultOutput) {
                    defaultDevice = (int)deviceIds.size() - 1;
                }
            }
        }
    }
}


AudioProber::AudioProber() :
    mBackends(),
    mMutex()
{
    QMutexLocker locker(&mMutex);

    std::vector<RtAudio::Api> apis;
    RtAudio::getCompiledApi(apis);

    for (auto api : apis) {
        mBackends.emplace_back(api);
    }
}

int AudioProber::getDefaultDevice(int backendIndex) const {
    QMutexLocker locker(&mMutex);

    if (indexIsInvalid(backendIndex)) {
        return 0;
    }

    return mBackends[backendIndex].defaultDevice;
}

int AudioProber::indexOfApi(RtAudio::Api api) const {
    QMutexLocker locker(&mMutex);

    int index = 0;
    for (auto const& backend : mBackends) {
        if (backend.api == api) {
            return index;
        }
        ++index;
    }
    return -1;

}

int AudioProber::indexOfDevice(int backendIndex, QString const& name) const {
    QMutexLocker locker(&mMutex);

    if (indexIsInvalid(backendIndex)) {
        return -1;
    }

    return mBackends[backendIndex].deviceNames.indexOf(name);
}

Guarded<RtAudio>* AudioProber::backend(int backendIndex) {
    QMutexLocker locker(&mMutex);

    if (indexIsInvalid(backendIndex)) {
        return nullptr;
    }

    return mBackends[backendIndex].get();
}

RtAudio::Api AudioProber::backendApi(int backendIndex) const {
    QMutexLocker locker(&mMutex);
    
    if (indexIsInvalid(backendIndex)) {
        return RtAudio::UNSPECIFIED;
    }

    return mBackends[backendIndex].api;
}

QStringList AudioProber::backendNames() const {
    QMutexLocker locker(&mMutex);

    QStringList list;
    for (auto &backend : mBackends) {
        auto name = RtAudio::getApiDisplayName(backend.api);
        list.append(QString::fromStdString(name));
    }
    return list;
}

QStringList AudioProber::deviceNames(int backendIndex) const {
    QMutexLocker locker(&mMutex);

    if (indexIsInvalid(backendIndex)) {
        return {};
    }

    return mBackends[backendIndex].deviceNames;
}

int AudioProber::findDevice(int backendIndex, QString const& name) const {
    QMutexLocker locker(&mMutex);

    if (indexIsInvalid(backendIndex)) {
        return -1;
    }

    return mBackends[backendIndex].deviceNames.indexOf(name);
}

unsigned AudioProber::mapDeviceIndex(int backendIndex, int deviceIndex) const {
    QMutexLocker locker(&mMutex);
    
    if (indexIsInvalid(backendIndex)) {
        return 0;
    }

    auto const& map = mBackends[backendIndex].deviceIds;
    if (deviceIndex < 0 || deviceIndex >= (int)map.size()) {
        return 0;
    }

    return map[deviceIndex];
}

AudioProber& AudioProber::instance() {
    static AudioProber prober;
    return prober;
}

void AudioProber::probe(int backendIndex) {
    QMutexLocker locker(&mMutex);

    if (indexIsInvalid(backendIndex)) {
        return;
    }

    mBackends[backendIndex].probe();
}

bool AudioProber::indexIsInvalid(int backendIndex) const {
    return backendIndex < 0 || backendIndex > (int)mBackends.size();
}
