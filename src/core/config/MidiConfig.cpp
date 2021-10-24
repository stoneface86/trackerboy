
#include "core/config/MidiConfig.hpp"

#include "core/config/keys.hpp"
#include "core/midi/MidiProber.hpp"

#include <QtDebug>

#define TU MidiConfigTU
namespace TU {

static const auto LOG_PREFIX = "[MidiConfig]";

}



MidiConfig::MidiConfig() :
    mEnabled(false),
    mBackendIndex(-1),
    mPortIndex(-1)
{
}

bool MidiConfig::isEnabled() const {
    return mEnabled;
}


int MidiConfig::backendIndex() const {
    return mBackendIndex;
}

int MidiConfig::portIndex() const {
    return mPortIndex;
}

void MidiConfig::setEnabled(bool enabled) {
    mEnabled = enabled;
}

void MidiConfig::setBackendIndex(int index) {
    if (index >= -1) {
        mBackendIndex = index;
    }
}

void MidiConfig::setPortIndex(int index) {
    if (index >= -1) {
        mPortIndex = index;
    }
}

void MidiConfig::readSettings(QSettings &settings) {
    settings.beginGroup(Keys::Midi);
    mEnabled = settings.value(Keys::enabled, mEnabled).toBool();


    QString apiName = settings.value(Keys::api).toString();
    auto api = RtMidi::getCompiledApiByName(apiName.toStdString());
    
    bool checkDevice = true;

    auto &prober = MidiProber::instance();
    if (api == RtMidi::UNSPECIFIED) {
        mBackendIndex = 0; // default to first available
        checkDevice = false;
    } else {
        mBackendIndex = prober.indexOfApi(api);
        if (mBackendIndex == -1) {
            // this should rarely happen
            qWarning() << TU::LOG_PREFIX << "MIDI API is not available, falling back to the first available";
            mBackendIndex = 0; // default to the first one
            checkDevice = false;
        }
    }
    prober.setBackend(mBackendIndex);

    if (checkDevice) {
        QString deviceName = settings.value(Keys::deviceName).toString();
        if (deviceName.isEmpty()) {
            mPortIndex = -1;
        } else {
            // we don't really have a way to uniquely identify the device so just use the
            // port name.
            mPortIndex = prober.portNames().indexOf(deviceName);
            // if portIndex is -1, then we couldn't find the port, log a warning
            if (mPortIndex == -1) {
                qWarning() << TU::LOG_PREFIX << "Could not find MIDI port, please select a new device";
            }
        }
    }

    settings.endGroup();


}

void MidiConfig::writeSettings(QSettings &settings) const {
    settings.beginGroup(Keys::Midi);
    settings.remove(QString());

    settings.setValue(Keys::enabled, mEnabled);

    auto &prober = MidiProber::instance();

    auto const names = prober.backendNames();
    QString api;
    if (mBackendIndex >= 0 && mBackendIndex < names.size()) {
        api = names[mBackendIndex];
    }
    settings.setValue(Keys::api, api);

    QString device;
    if (mPortIndex != -1) {
        device = prober.portNames().at(mPortIndex);
    }
    settings.setValue(Keys::deviceName, device);
    
    settings.endGroup();
}

#undef TU
