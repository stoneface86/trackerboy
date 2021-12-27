
#pragma once

#include <QFont>
#include <QSettings>


class AppearanceConfig {

public:

    AppearanceConfig();

    QFont patternGridFont() const;

    QFont patternGridHeaderFont() const;

    QFont orderGridFont() const;

    void setPatternGridFont(QFont const& font);

    void setPatternGridHeaderFont(QFont const& font);

    void setOrderGridFont(QFont const& font);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    QFont mPatternGridFont;
    QFont mPatternGridHeaderFont;
    QFont mOrderGridFont;

};
