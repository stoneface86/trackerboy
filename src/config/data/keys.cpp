
#include "config/data/keys.hpp"


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
QString const autosave { QStringLiteral("autosave") };
QString const autosaveInterval { QStringLiteral("autosaveInterval") };
QString const backupCopy { QStringLiteral("backupCopy") };
QString const bindingsLower { QStringLiteral("bindingsLower") };
QString const bindingsUpper { QStringLiteral("bindingsUpper") };
QString const cursorWrap { QStringLiteral("cursorWrap") };
QString const cursorWrapPattern { QStringLiteral("cursorWrapPattern") };
QString const deviceName { QStringLiteral("deviceName") };
QString const enabled { QStringLiteral("enabled") };
QString const showFlats { QStringLiteral("showFlats") };
QString const showPreviews { QStringLiteral("showPreviews") };
QString const pageStep { QStringLiteral("pageStep") };
QString const patternGridFont { QStringLiteral("patternGridFont") };
QString const orderGridFont { QStringLiteral("orderGridFont") };
QString const patternGridHeaderFont { QStringLiteral("patternGridHeaderFont") };
QString const keyboardLayout { QStringLiteral("keyboardLayout") };
QString const key { QStringLiteral("key") };
QString const rownoHex ( QStringLiteral("rownoHex") );
QString const samplerate { QStringLiteral("samplerate") };
QString const period { QStringLiteral("period") };
QString const latency { QStringLiteral("latency") };
QString const deviceId { QStringLiteral("deviceId") };
QString const noteCut { QStringLiteral("noteCut") };


}
