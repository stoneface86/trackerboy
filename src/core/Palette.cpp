
#include "core/Palette.hpp"

#include "core/config/keys.hpp"

#include <QtDebug>

#include <array>

Palette::Palette() :
    mData{
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
        QColor(217,  87,  99)  // ColorHeaderDisabled
    },
    mDefault(true)
{

}

QColor const& Palette::operator[](Color color) const {
    return mData[color];
}

bool Palette::isDefault() const {
    return mDefault;
}

void Palette::setColor(Color color, QColor value) {
    mData[color] = value;
    mDefault = false;
}

//
// QString interning for QSettings, QMetaEnum was not used because:
//  * requires QString conversion
//  * enum names are not human friendly (background vs ColorBackground)
//  * requires MOC
//
// downside to this is that we need a matching key string for each Color enum
//
static std::array const ColorKeys = {
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
    QStringLiteral("headerDisabled")
};

// If this fails you forgot to remove/add a key to the array
static_assert(ColorKeys.size() == Palette::ColorCount, "ColorKeys.size() does not match enum count!");


void Palette::readSettings(QSettings &settings) {

    // QSettings has no way of checking if a group exists, so we have to read
    // all color keys even if the user is using the default :(

    settings.beginGroup(Keys::Palette);

    for (int i = 0; i < ColorCount; ++i) {
        if (settings.contains(ColorKeys[i])) {
            QColor color(settings.value(ColorKeys[i]).toString());
            if (color.isValid()) {
                color.setAlpha(255); // sanitize
                setColor((Color)i, color);
            }
        }
    }

    settings.endGroup();
}

void Palette::writeSettings(QSettings &settings, bool saveOnDefault) const {

    settings.beginGroup(Keys::Palette);
    settings.remove(QString()); // remove everything

    if (saveOnDefault || !mDefault) {
        for (int i = 0; i < ColorCount; ++i) {
            QString color = mData[i].name();
            settings.setValue(ColorKeys[i], color);
        }
    }

    settings.endGroup();

}

bool operator==(Palette const& lhs, Palette const& rhs) noexcept {
    if (lhs.mDefault == rhs.mDefault) {
        if (lhs.mDefault) {
            return true;
        } else {
            return lhs.mData == rhs.mData;
        }
    }
    return false;
}

bool operator!=(Palette const& lhs, Palette const& rhs) noexcept {
    return !(lhs == rhs);
}
