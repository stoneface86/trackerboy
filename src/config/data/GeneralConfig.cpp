
#include "config/data/GeneralConfig.hpp"

#include "config/data/keys.hpp"

GeneralConfig::GeneralConfig() :
    mHistoryLimit(64),
    mAutosave(false),
    mAutosaveInterval(1),
    mOptions()
{
}

int GeneralConfig::historyLimit() const {
    return mHistoryLimit;
}

void GeneralConfig::setHistoryLimit(int limit) {
    if (limit >= 0) {
        mHistoryLimit = limit;
    }
}

bool GeneralConfig::hasAutosave() const {
    return mAutosave;
}

void GeneralConfig::setAutosave(bool autosave) {
    mAutosave = autosave;
}

int GeneralConfig::autosaveInterval() const {
    return mAutosaveInterval;
}

void GeneralConfig::setAutosaveInterval(int interval) {
    mAutosaveInterval = interval;
}

bool GeneralConfig::hasOption(Options option) const {
    return mOptions.test(option);
}

void GeneralConfig::setOption(Options option, bool enabled) {
    mOptions.set(option, enabled);
}

void GeneralConfig::readSettings(QSettings &settings) {
    settings.beginGroup(Keys::General);

    mHistoryLimit = settings.value(Keys::historyLimit, mHistoryLimit).toInt();
    
    auto readOption = [this, &settings](Options option, QString const& key, bool defaultValue) {
        mOptions.set(option, settings.value(key, defaultValue).toBool());
    };
    readOption(OptionBackupCopy, Keys::backupCopy, false);
    readOption(OptionCursorWrap, Keys::cursorWrap, true);
    readOption(OptionCursorWrapPattern, Keys::cursorWrapPattern, true);
    readOption(OptionRownoHex, Keys::rownoHex, true);

    // autosave is off by default
    mAutosave = settings.value(Keys::autosave, false).toBool();
    // default autosave interval is 30 seconds
    mAutosaveInterval = settings.value(Keys::autosaveInterval, 30).toInt();

    settings.endGroup();
}

void GeneralConfig::writeSettings(QSettings &settings) const {
    settings.beginGroup(Keys::General);
    settings.remove(QString());

    settings.setValue(Keys::historyLimit, mHistoryLimit);
    settings.setValue(Keys::autosave, mAutosave);
    settings.setValue(Keys::autosaveInterval, mAutosaveInterval);
    auto writeOption = [this, &settings](Options option, QString const& key) {
        settings.setValue(key, mOptions.test(option));
    };
    writeOption(OptionBackupCopy, Keys::backupCopy);
    writeOption(OptionCursorWrap, Keys::cursorWrap);
    writeOption(OptionCursorWrapPattern, Keys::cursorWrapPattern);
    writeOption(OptionRownoHex, Keys::rownoHex);

    settings.endGroup();
}
