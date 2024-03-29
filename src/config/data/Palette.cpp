
#include "config/data/Palette.hpp"

#include "config/data/keys.hpp"

#include <QtDebug>

#define TU PaletteTU

Palette::Palette() :
    mData()
{
}

QColor const& Palette::operator[](Color color) const {
    return mData[color];
}

namespace TU {

static std::array<QColor, Palette::ColorCount> const ColorDefaults = {
    QColor( 24,  24,  24), // ColorBackground
    QColor( 32,  32,  32), // ColorBackgroundHighlight1
    QColor( 48,  48,  48), // ColorBackgroundHighlight2
    QColor(192, 192, 192), // ColorForeground
    QColor(240, 240, 240), // ColorForegroundHighlight1
    QColor(255, 255, 255), // ColorForegroundHighlight2
    QColor( 32,  48, 128), // ColorRow
    QColor(128,  48,  48), // ColorRowEdit
    QColor(128,  32, 128), // ColorRowPlayer
    QColor(128, 128, 255), // ColorEffectType
    QColor(128, 255, 128), // ColorInstrument
    QColor( 69,  69,  80), // ColorSelection
    QColor(192, 192, 192), // ColorCursor
    QColor( 64,  64,  64), // ColorLine
    QColor( 48,  48,  48), // ColorHeaderBackground1
    QColor( 40,  40,  40), // ColorHeaderBackground2
    QColor(240, 240, 240), // ColorHeaderForeground1
    QColor(192, 192, 192), // ColorHeaderForeground2
    QColor(153, 229,  80), // ColorHeaderEnabled
    QColor(217,  87,  99), // ColorHeaderDisabled
    QColor(  0,   0,   0), // ColorGraphBackground
    QColor( 32,  32,  32), // ColorGraphAlternate
    QColor( 64,  64,  64), // ColorGraphLines
    QColor(224, 224, 224), // ColorGraphSamples
    QColor(  0,   0,   0), // ColorScopeBackground
    QColor(  0, 255,  68)  // ColorScopeLine
};

}

QColor Palette::getDefault(Color color) {
    return TU::ColorDefaults[color];
}

void Palette::setColor(Color color, QColor value) {
    mData[color] = value;
}

namespace TU {

//
// QString interning for QSettings, QMetaEnum was not used because:
//  * requires QString conversion
//  * enum names are not human friendly (background vs ColorBackground)
//  * requires MOC
//
// downside to this is that we need a matching key string for each Color enum
//
static std::array<QString, Palette::ColorCount> const ColorKeys = {
    QStringLiteral("background"),
    QStringLiteral("backgroundHighlight1"),
    QStringLiteral("backgroundHighlight2"),
    QStringLiteral("foreground"),
    QStringLiteral("foregroundHighlight1"),
    QStringLiteral("foregroundHighlight2"),
    QStringLiteral("row"),
    QStringLiteral("rowEdit"),
    QStringLiteral("rowPlayer"),
    QStringLiteral("effectType"),
    QStringLiteral("instrument"),
    QStringLiteral("selection"),
    QStringLiteral("cursor"),
    QStringLiteral("line"),
    QStringLiteral("headerBackground1"),
    QStringLiteral("headerBackground2"),
    QStringLiteral("headerForeground1"),
    QStringLiteral("headerForeground2"),
    QStringLiteral("headerEnabled"),
    QStringLiteral("headerDisabled"),
    QStringLiteral("graphBackground"),
    QStringLiteral("graphAlternate"),
    QStringLiteral("graphLines"),
    QStringLiteral("graphSamples"),
    QStringLiteral("scopeBackground"),
    QStringLiteral("scopeLine")
};

}

void Palette::readSettings(QSettings &settings) {

    settings.beginGroup(Keys::Palette);

    for (int i = 0; i < ColorCount; ++i) {

        auto colorstr = settings.value(TU::ColorKeys[i]).toString();
        if (!colorstr.isEmpty()) {
            QColor color(colorstr);
            if (color.isValid()) {
                color.setAlpha(255); // sanitize
                mData[i] = color;
                continue; // success, do not load default
            }
        }
        // setting not found or was invalid
        // load the default
        mData[i] = getDefault((Color)i);
    }

    settings.endGroup();
}

void Palette::writeSettings(QSettings &settings) const {

    settings.beginGroup(Keys::Palette);
    settings.remove(QString()); // remove everything

    for (int i = 0; i < ColorCount; ++i) {
        QString color = mData[i].name();
        settings.setValue(TU::ColorKeys[i], color);
    }

    settings.endGroup();

}

bool operator==(Palette const& lhs, Palette const& rhs) noexcept {
    return lhs.mData == rhs.mData;
}

bool operator!=(Palette const& lhs, Palette const& rhs) noexcept {
    return !(lhs == rhs);
}

#undef TU
