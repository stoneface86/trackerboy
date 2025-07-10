
#pragma once

#include <QImage>
#include <QPixmap>
#include <QRgb>
#include <QString>

#include <array>

///
/// A structure containing 4 colors for theming icons and other ui widgets.
///
struct ColorTheme {

    ///
    /// Indices for each color in the theme
    ///
    enum Color { ColorLightest, ColorLight, ColorDark, ColorDarkest };

    ///
    /// Total number of colors available in this theme.
    ///
    static constexpr auto ColorCount = 4;

    ///
    /// Color data as an array of QRgb. The first color should be the lightest
    /// shade and the last should be the darkest.
    ///
    std::array<QRgb, ColorCount> colors;

    ///
    /// Sets the given image's color table to this theme. `image` is expected
    /// to have an image format of `QImage::Format_Indexed8`.
    ///
    void colorize(QImage &image) const;

    ///
    /// Loads an image file and colorizes with this theme, returning the result
    /// as a `QPixmap` to be displayed on the screen.
    ///
    QPixmap colorizeFile(QString const &file) const;
};
