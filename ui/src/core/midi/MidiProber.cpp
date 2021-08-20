
#include "core/midi/MidiProber.hpp"

#include <QtDebug>

#include <algorithm>

static auto CLIENT_NAME = "Trackerboy input client";


MidiProber::MidiProber() :
    mApiVec(),
    mBackendNames(),
    mPortNames(),
    mBackendIndex(-1),
    mBackend(nullptr)
{
    RtMidi::getCompiledApi(mApiVec);

    for (auto api : mApiVec) {
        mBackendNames.append(QString::fromStdString(RtMidi::getApiDisplayName(api)));
    }

}

int MidiProber::indexOfApi(RtMidi::Api api) const {
    auto iter = std::find(mApiVec.begin(), mApiVec.end(), api);
    if (iter == mApiVec.end()) {
        return -1;
    } else {
        return iter - mApiVec.begin();
    }
}

RtMidi::Api MidiProber::api() const {
    return mBackendIndex == -1 ? RtMidi::UNSPECIFIED : mApiVec[mBackendIndex];
}


QStringList MidiProber::backendNames() const {
    return mBackendNames;
}

QStringList MidiProber::portNames() const {
    return mPortNames;
}

void MidiProber::setBackend(int index) {

    if (mBackendIndex != index) {
        mBackendIndex = index;
        if (index < 0 || index >= (int)mApiVec.size()) {
            noBackend();
        } else {
            try {
                mBackend = std::make_shared<RtMidiIn>(mApiVec[index], CLIENT_NAME);
            } catch (RtMidiError const &err) {
                qCritical().noquote() << "[MIDI] Couldn't initialize API:" << QString::fromStdString(err.getMessage());
                noBackend();
                return;
            }

            probe();
        }
    }
}

std::shared_ptr<RtMidiIn> MidiProber::backend() const {
    return mBackend;
}

void MidiProber::probe() {
    if (mBackend) {
        mPortNames.clear();
        try {
            auto count = mBackend->getPortCount();
            for (decltype(count) i = 0; i < count; ++i) {
                auto name = mBackend->getPortName(i);
                mPortNames.append(QString::fromStdString(name));
            }
        } catch (RtMidiError const& err) {
            qCritical().noquote() << "[MIDI] error occurred during probing:" << QString::fromStdString(err.getMessage());
            noBackend();
        }
    }
}


MidiProber& MidiProber::instance() {
    static MidiProber _instance;

    return _instance;
}

void MidiProber::noBackend() {
    mBackend = nullptr;
    mPortNames.clear();
}
