
#pragma once

#include <QColor>
#include <QDataStream>
#include <QSettings>

#include <array>

//
// Container class for all colors used by the application. Not to be confused
// with QPalette.
//
class Palette {

public:

    enum Color {
        // background color
        ColorBackground,

        // background color for first highlight (beat row)
        ColorBackgroundHighlight1,

        // background color for second highlight (measure row)
        ColorBackgroundHighlight2,

        // general text color
        ColorForeground,

        // text color for first highlight (beat row)
        ColorForegroundHighlight1,

        // text color for second highlight (measure row)
        ColorForegroundHighlight2,

        // background color for the cursor row
        ColorRow,

        // same as row, but edit mode is enabled
        ColorRowEdit,

        // cursor row of the renderer (only visible when follow mode is off)
        ColorRowPlayer,

        // text color for effect column
        ColorEffectType,

        // text color for instrument column
        ColorInstrument,

        // selection rectangle color
        ColorSelection,

        // cursor rectangle color
        ColorCursor,

        ColorLine,

        ColorHeaderBackground,
        ColorHeaderForeground,
        ColorHeaderHover,
        ColorHeaderDisabled,

        ColorCount
    };

    //
    // Initializes the palette with the default colors
    //
    Palette();

    QColor& operator[](Color color);
    QColor const& operator[](Color color) const;

    //
    // Reads color settings from the given QSettings in the "Palette" group
    //
    void readSettings(QSettings &settings);

    //
    // Writes color settings to the given QSettings.
    //
    void writeSettings(QSettings &settings) const;

private:

    std::array<QColor, ColorCount> mData;

};
