
#include "midi/MidiEnumerator.hpp"


MidiEnumerator::Device::Device() :
    api(RtMidi::UNSPECIFIED),
    portName()
{
}

MidiEnumerator::Device::Device(RtMidi::Api api, std::string &&name) :
    api(api),
    portName(std::move(name))
{
}

MidiEnumerator::Context::Context(RtMidi::Api api) :
    api(api),
    deviceNames(),
    available(false)
{
}


MidiEnumerator::MidiEnumerator() :
    mContexts()
{
    std::vector<RtMidi::Api> apis;
    RtMidi::getCompiledApi(apis);

    for (auto api : apis) {
        mContexts.emplace_back(api);
    }

}

bool MidiEnumerator::backendIsAvailable(int backend) const {
    if (indexIsInvalid(backend)) {
        return false;
    }
    return mContexts[backend].available;
}

QStringList MidiEnumerator::backendNames() const {
    QStringList names;
    for (auto &ctx : mContexts) {
        names.append(QString::fromStdString(RtMidi::getApiDisplayName(ctx.api)));
    }
    return names;
}

int MidiEnumerator::backends() const {
    return (int)mContexts.size();
}

QStringList MidiEnumerator::deviceNames(int backend) const {
    if (indexIsInvalid(backend)) {
        return {};
    }
    return mContexts[backend].deviceNames;
}

int MidiEnumerator::devices(int backend) const {
    if (indexIsInvalid(backend)) {
        return 0;
    }
    return mContexts[backend].deviceNames.count();
}

MidiEnumerator::Device MidiEnumerator::device(int backend, int device) const {
    if (indexIsInvalid(backend)) {
        return {};
    }

    auto &ctx = mContexts[backend];

    if (device < 0 || device >= (int)ctx.deviceNames.size()) {
        return {};
    }

    return {
        ctx.api,
        ctx.deviceNames[device].toStdString()
    };
}

QVariant MidiEnumerator::serializeDevice(int backend, int device) const {
    if (indexIsInvalid(backend)) {
        return QString();
    }
    auto &ctx = mContexts[backend];
    if (device < 0 || device >= ctx.deviceNames.count()) {
        return QString();
    }
    return ctx.deviceNames[device];
}

int MidiEnumerator::deserializeDevice(int backend, const QVariant &data) const {
    if (indexIsInvalid(backend)) {
        return -1;
    }

    auto name = data.toString();
    if (name.isEmpty()) {
        return -1;
    } else {
        return mContexts[backend].deviceNames.indexOf(name);
    }
}

void MidiEnumerator::populate(int backend) {
    if (indexIsInvalid(backend)) {
        return;
    }

    auto &ctx = mContexts[backend];
    ctx.deviceNames.clear();

    try {
        RtMidiIn midiIn(ctx.api);

        auto const portCount = midiIn.getPortCount();
        for (unsigned port = 0; port < portCount; ++port) {
            ctx.deviceNames.append(QString::fromStdString(midiIn.getPortName(port)));
        }

        ctx.available = true;
    }  catch (RtMidiError const& err) {
        ctx.available = false;
    }
}

bool MidiEnumerator::indexIsInvalid(int backend) const {
    return backend < 0 || backend >= (int)mContexts.size();
}
