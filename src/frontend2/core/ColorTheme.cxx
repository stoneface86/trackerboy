
#include "core/ColorTheme.hxx"

#include <utility>

ColorTheme::ColorTheme()
    : colors{qRgb(208, 208, 247), qRgb(113, 113, 191), qRgb(51, 51, 102),
             qRgb(31, 31, 61)} {}

void ColorTheme::colorize(QImage &image) const {
    for (auto i = 0; i < ColorCount; ++i) {
        image.setColor(i + 1, colors[i]);
    }
}

QPixmap ColorTheme::colorizeFile(QString const &file) const {
    QImage img(file);
    colorize(img);
    return QPixmap::fromImage(std::move(img));
}
