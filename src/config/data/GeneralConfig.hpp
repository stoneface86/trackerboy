
#pragma once

#include <QSettings>

class GeneralConfig {

public:

    GeneralConfig();

    int historyLimit() const;
    void setHistoryLimit(int limit);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    int mHistoryLimit;

};
