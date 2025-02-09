
#pragma once

#include <QFont>
#include <QSettings>

#include <array>

class FontTable {

public:

    enum Fonts {
        PatternGrid,
        PatternGridHeader,
        OrderGrid,

        Count
    };

    explicit FontTable();

    QFont const& operator[](Fonts font) const;

    void set(Fonts font, QFont const& val);

    void readSettings(QSettings &settings);

    void writeSettings(QSettings &settings) const;

private:

    std::array<QFont, Count> mFonts;

};
