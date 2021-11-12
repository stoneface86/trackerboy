
#include "core/config/MidiConfig.hpp"

#include "core/config/keys.hpp"
#include "midi/MidiEnumerator.hpp"

#include <QtDebug>
#include <QSettings>

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

void MidiConfig::readSettings(QSettings &settings, MidiEnumerator &enumerator) {
    settings.beginGroup(Keys::Midi);
    mEnabled = settings.value(Keys::enabled, false).toBool();


    auto apiName = settings.value(Keys::api).toString();
    if (apiName.isEmpty()) {
        mBackendIndex = 0;
        mPortIndex = -1;
    } else {
        mBackendIndex = enumerator.backendNames().indexOf(apiName);
        if (mBackendIndex == -1) {
            qWarning() << TU::LOG_PREFIX << "MIDI API is not available, falling back to the first available";
            mBackendIndex = 0;
            mPortIndex = -1;
        } else {
            auto device = settings.value(Keys::deviceName);
            enumerator.populate(mBackendIndex);
            mPortIndex = enumerator.deserializeDevice(mBackendIndex, device);
            if (mPortIndex == -1) {
                qWarning() << TU::LOG_PREFIX << "Could not find MIDI port, please select a new device";
            }
        }
    }

    settings.endGroup();

}

void MidiConfig::writeSettings(QSettings &settings, MidiEnumerator const& enumerator) const {
    settings.beginGroup(Keys::Midi);
    settings.remove(QString());

    settings.setValue(Keys::enabled, mEnabled);

    settings.setValue(Keys::api, enumerator.backendNames().at(mBackendIndex));
    settings.setValue(Keys::deviceName, enumerator.serializeDevice(mBackendIndex, mPortIndex));
    
    settings.endGroup();
}

#undef TU
