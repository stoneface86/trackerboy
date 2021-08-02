
#pragma once

#include <QSettings>

class MidiConfig {

public:

    MidiConfig();

    bool isEnabled() const;

    int backendIndex() const;

    int portIndex() const;

    void setEnabled(bool enabled);

    void setBackendIndex(int index);

    void setPortIndex(int index);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    bool mEnabled;
    int mBackendIndex;
    int mPortIndex;


};
