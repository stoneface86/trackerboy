
#include "core/ColorTheme.hxx"

#include <utility>

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
