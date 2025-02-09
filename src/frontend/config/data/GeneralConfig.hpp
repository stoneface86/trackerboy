
#pragma once

#include <QSettings>

#include <bitset>

class GeneralConfig {

public:

    enum Options {
        OptionBackupCopy,
        OptionCursorWrap,
        OptionCursorWrapPattern,    // wrap the cursor to start/end of order
        OptionRownoHex,             // use hex for row numbers
        OptionShowFlats,            // if enabled flats will be shown for accidental notes
        OptionShowPreviews,         // if enabled previous/next pattern in order will be drawn

        OptionCount
    };

    GeneralConfig();

    int pageStep() const;
    void setPageStep(int step);

    bool hasAutosave() const;
    void setAutosave(bool autosave);

    int autosaveInterval() const;
    void setAutosaveInterval(int interval);

    bool hasOption(Options option) const;
    void setOption(Options option, bool enabled);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    int mPageStep;

    bool mAutosave;
    int mAutosaveInterval;

    std::bitset<OptionCount> mOptions;

};
