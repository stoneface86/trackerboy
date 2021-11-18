
#pragma once

#include <QFont>
#include <QSettings>


class AppearanceConfig {

public:

    AppearanceConfig();

    bool showFlats() const;

    bool showPreviews() const;

    QFont patternGridFont() const;

    QFont patternGridHeaderFont() const;

    QFont orderGridFont() const;

    void setPatternGridFont(QFont const& font);

    void setPatternGridHeaderFont(QFont const& font);

    void setOrderGridFont(QFont const& font);

    void setShowFlats(bool showFlats);

    void setShowPreviews(bool showPreviews);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    QFont mPatternGridFont;
    QFont mPatternGridHeaderFont;
    QFont mOrderGridFont;
    bool mShowFlats;         // if true flats will be shown for accidental notes
    bool mShowPreviews;      // if true, pattern previews will be rendered

};
