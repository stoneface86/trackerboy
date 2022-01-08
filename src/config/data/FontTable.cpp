
#include "config/data/FontTable.hpp"
#include "config/data/keys.hpp"

#include <QFontDatabase>

#include <optional>

FontTable::FontTable() :
    mFonts()
{
}

QFont const& FontTable::operator[](Fonts font) const {
    return mFonts[font];
}

void FontTable::set(Fonts font, QFont const& val) {
    mFonts[font] = val;
}

#define TU FontTableTU
namespace TU {

static std::array<QString, FontTable::Count> const FontKeys = {
    QStringLiteral("patternGrid"),
    QStringLiteral("patternGridHeader"),
    QStringLiteral("orderGrid")
};

}


void FontTable::readSettings(QSettings &settings) {

    settings.beginGroup(Keys::Fonts);

    std::optional<QFont> defaultFont;

    for (int i = 0; i < Count; ++i) {
        auto fontstr = settings.value(TU::FontKeys[i]).toString();
        auto &font = mFonts[i];
        if (fontstr.isEmpty() || !font.fromString(fontstr)) {
            if (!defaultFont) {
                // use the system default fixed-width font
                defaultFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
                defaultFont->setPointSize(12);
            }
            font = *defaultFont;
        }
    }

    settings.endGroup();

}

void FontTable::writeSettings(QSettings &settings) const {

    settings.beginGroup(Keys::Fonts);

    for (int i = 0; i < Count; ++i) {
        settings.setValue(TU::FontKeys[i], mFonts[i].toString());
    }

    settings.endGroup();
}

#undef TU
