#include "core/settings.hxx"

#include <QDir>
#include <QStandardPaths>

Settings::Settings(SettingsKind k, QString const &group)
    : QSettings(path(k), QSettings::IniFormat) {
    beginGroup(group);
}

const char *filename(SettingsKind k) {
    switch (k) {
    case SettingsState:
        return "state.ini";
    case SettingsKeybinds:
        return "keybinds.ini";
    default:
        return "config.ini";
    }
}

QString path(SettingsKind k) {
    QDir configPath(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    return configPath.filePath(filename(k));
}

namespace lit {

#define KEY(str) strlit str = #str

KEY(size);
KEY(state);
KEY(geometry);

#undef KEY
} // namespace lit