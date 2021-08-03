
#include "core/Palette.hpp"

#include "core/config/keys.hpp"

#include <QtDebug>

#include <array>

Palette::Palette() :
    mData{
        QColor( 24,  24,  24),
        QColor( 32,  32,  32),
        QColor( 48,  48,  48),
        QColor(192, 192, 192),
        QColor(240, 240, 240),
        QColor(255, 255, 255),
        QColor( 32,  48, 128),
        QColor(128,  48,  48),
        QColor(128,  32, 128),
        QColor(128, 128, 255),
        QColor(128, 255, 128),
        QColor( 69,  69,  80),
        QColor(192, 192, 192),
        QColor( 64,  64,  64),
        QColor(224, 248, 208),
        QColor(  8,  24,  32),
        QColor(136, 192, 112),
        QColor( 52, 104,  86)
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
    QStringLiteral("headerBackground"),
    QStringLiteral("headerforeground"),
    QStringLiteral("headerhover"),
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
            setColor((Color)i, color);            
        }
    }

    settings.endGroup();
}

void Palette::writeSettings(QSettings &settings) const {
    if (mDefault) {
        return; // no need to save palette when it's the default
    }

    settings.beginGroup(Keys::Palette);
    settings.remove(QString()); // remove everything

    for (int i = 0; i < ColorCount; ++i) {
        QString color = mData[i].name();
        settings.setValue(ColorKeys[i], color);
    }

    settings.endGroup();

}
