
#pragma once

#include <QSettings>

#include <bitset>

class GeneralConfig {

public:

    enum Options {
        OptionBackupCopy,
        OptionCursorWrap,
        OptionCursorWrapPattern,
        OptionRownoHex,

        OptionCount
    };

    GeneralConfig();

    int historyLimit() const;
    void setHistoryLimit(int limit);

    bool hasAutosave() const;
    void setAutosave(bool autosave);

    int autosaveInterval() const;
    void setAutosaveInterval(int interval);

    bool hasOption(Options option) const;
    void setOption(Options option, bool enabled);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    int mHistoryLimit;

    bool mAutosave;
    int mAutosaveInterval;

    std::bitset<OptionCount> mOptions;

};
