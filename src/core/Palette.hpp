
#pragma once

#include <QColor>
#include <QSettings>

#include <array>

//
// Container class for all colors used by the application. Not to be confused
// with QPalette.
//
class Palette {

public:

    enum Color {

        // Pattern editor colors

        ColorBackground,                // background color
        ColorBackgroundHighlight1,      // background color for first highlight (beat row)
        ColorBackgroundHighlight2,      // background color for second highlight (measure row)
        ColorForeground,                // general text color
        ColorForegroundHighlight1,      // text color for first highlight (beat row)
        ColorForegroundHighlight2,      // text color for second highlight (measure row)
        ColorRow,                       // background color for the cursor row
        ColorRowEdit,                   // same as row, but edit mode is enabled
        ColorRowPlayer,                 // cursor row of the renderer (only visible when follow mode is off)
        ColorEffectType,                // text color for effect column
        ColorInstrument,                // text color for instrument column
        ColorSelection,                 // selection rectangle color
        ColorCursor,                    // cursor rectangle color
        ColorLine,

        // Pattern editor header

        ColorHeaderBackground1,
        ColorHeaderBackground2,
        ColorHeaderForeground1,
        ColorHeaderForeground2,
        ColorHeaderEnabled,
        ColorHeaderDisabled,

        // Graph editor

        ColorGraphBackground,
        ColorGraphAlternate,
        ColorGraphLines,
        ColorGraphSamples,

        // Audio scope

        ColorScopeBackground,
        ColorScopeLine,


        ColorCount
    };

    //
    // Initializes the palette with the default colors
    //
    Palette();

    //
    // Access a color setting for the given color index
    //
    QColor const& operator[](Color color) const;

    //
    // Returns true if this palette is the default, or has not had any of its
    // colors modified.
    //
    bool isDefault() const;

    //
    // Sets the color for the given color setting. Calling this method
    // clears the default property.
    //
    void setColor(Color color, QColor value);

    //
    // Reads color settings from the given QSettings in the "Palette" group
    //
    void readSettings(QSettings &settings);

    //
    // Writes color settings to the given QSettings. Settings are not saved
    // when the palette is the default one, unless saveOnDefault is true.
    //
    void writeSettings(QSettings &settings, bool saveOnDefault = false) const;

    friend bool operator==(Palette const& lhs, Palette const& rhs) noexcept;
    friend bool operator!=(Palette const& lhs, Palette const& rhs) noexcept;

private:

    std::array<QColor, ColorCount> mData;
    bool mDefault;

};
