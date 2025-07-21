//
// Application settings management
//

#pragma once

#include "utils/aliases.hxx"

#include <QSettings>

//
// Enumeration of settings categories
//
enum SettingsKind {
    SettingsState,    // Window state, geometry, etc
    SettingsKeybinds, // Keyboard shortcut overrides
    SettingsConfig    // Application configuration
};

const char *filename(SettingsKind k);

//
// Get the path of the configuration file for the given settings category.
//
QString path(SettingsKind k);

//
// Wrapper for QSettings that adds a custom constructor
//
class Settings : public QSettings {

public:
    Settings(SettingsKind k, QString const &group);

private:
    Q_DISABLE_COPY(Settings)
};

//
// String literals to be used as keys when accessing values in a QSettings
//
namespace lit {

extern strlit size;
extern strlit state;
extern strlit geometry;

} // namespace lit