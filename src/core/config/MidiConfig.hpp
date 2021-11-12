
#pragma once

class MidiEnumerator;

class QSettings;

class MidiConfig {

public:

    MidiConfig();

    bool isEnabled() const;

    int backendIndex() const;

    int portIndex() const;

    void setEnabled(bool enabled);

    void setBackendIndex(int index);

    void setPortIndex(int index);

    void readSettings(QSettings &settings, MidiEnumerator &enumerator);

    void writeSettings(QSettings &settings, MidiEnumerator const& enumerator) const;

private:

    bool mEnabled;
    int mBackendIndex;
    int mPortIndex;


};
