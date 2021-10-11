
#include "core/config/keys.hpp"


namespace Keys {

// can't use a macro since QStringLiteral is a macro
// so unfortunately we have to write everything out manually

QString const Appearance { QStringLiteral("Appearance") };
QString const General { QStringLiteral("General") };
QString const Midi { QStringLiteral("Midi") };
QString const Palette { QStringLiteral("Palette") };
QString const PianoInput { QStringLiteral("PianoInput") };
QString const Sound { QStringLiteral("Sound") };

QString const api { QStringLiteral("api") };
QString const bindings { QStringLiteral("bindings") };
QString const deviceName { QStringLiteral("deviceName") };
QString const enabled { QStringLiteral("enabled") };
QString const showFlats { QStringLiteral("showFlats") };
QString const showPreviews { QStringLiteral("showPreviews") };
QString const font { QStringLiteral("font") };
QString const fontSize { QStringLiteral("fontSize") };
QString const historyLimit { QStringLiteral("historyLimit") };
QString const keyboardLayout { QStringLiteral("keyboardLayout") };
QString const key { QStringLiteral("key") };
QString const samplerate { QStringLiteral("samplerate") };
QString const period { QStringLiteral("period") };
QString const latency { QStringLiteral("latency") };
QString const deviceId { QStringLiteral("deviceId") };

}
