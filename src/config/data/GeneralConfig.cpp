
#include "config/data/GeneralConfig.hpp"

#include "config/data/keys.hpp"

GeneralConfig::GeneralConfig() :
    mHistoryLimit(64)
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

void GeneralConfig::readSettings(QSettings &settings) {
    settings.beginGroup(Keys::General);

    mHistoryLimit = settings.value(Keys::historyLimit, mHistoryLimit).toInt();

    settings.endGroup();
}

void GeneralConfig::writeSettings(QSettings &settings) const {
    settings.beginGroup(Keys::General);
    settings.remove(QString());

    settings.setValue(Keys::historyLimit, mHistoryLimit);

    settings.endGroup();
}
