
// Table of QColors used throughout the application

#pragma once

#include <QColor>

#include <array>
#include <type_traits>


// color indices
enum class Color {
    // background color
    background,

    // background color for first highlight (beat row)
    backgroundHighlight1,

    // background color for second highlight (measure row)
    backgroundHighlight2,

    // general text color
    foreground,

    // text color for first highlight (beat row)
    foregroundHighlight1,

    // text color for second highlight (measure row)
    foregroundHighlight2,

    // background color for the cursor row
    row,

    // same as row, but edit mode is enabled
    rowEdit,

    // cursor row of the renderer (only visible when follow mode is off)
    rowPlayer,

    // text color for effect column
    effectType,

    // text color for instrument column
    instrument,

    // selection rectangle color
    selection,

    // cursor rectangle color
    cursor,

    line,

    headerBackground,
    headerForeground,
    headerHover,
    headerDisabled,


    // alias

    orderForeground = foreground,
    orderBackground = background,
    orderCursor = cursor,


    

    __last = headerDisabled
};


using ColorTable = std::array<QColor, static_cast<size_t>(Color::__last) + 1>;

// utility overload converts Color enum to its underlying integer type
// so we can do +Color::line instead of static_cast<int>(Color::line)
inline constexpr auto operator+(Color color) noexcept {
    return static_cast<std::underlying_type_t<Color>>(color);
}


