
#include "core/icons.hxx"

#include <utility>

#define TU IconLocatorTU
namespace TU {

static constexpr auto cSmallIconOnly = icons::Ch1;

class IconTable {

public:
    void generate(ColorTheme const &theme) {
        // Icons are stored in a single indexed PNG image
        // Colored using 5 colors:
        // - Index 0: transparent
        // - Index 1-4: indices for theme's colors
        auto tilesetPix = theme.colorizeFile(":/img/icons.png");

        constexpr auto cTileSmall = icons::smallSize.width();
        constexpr auto cTileLarge = icons::largeSize.width();
        // large tiles start after three rows of small tiles
        constexpr auto cLargeStartY = cTileSmall * 3;

        int smallX = 0;
        int smallY = 0;
        int largeX = 0;
        int largeY = cLargeStartY;
        auto const tilesetWidth = tilesetPix.width();

        auto addTile = [](QIcon &icon, int &x, int &y, int const tileSize,
                          int const tilesetWidth, QPixmap const &tileset) {
            icon.addPixmap(tileset.copy(x, y, tileSize, tileSize));
            x += tileSize;
            icon.addPixmap(tileset.copy(x, y, tileSize, tileSize),
                           QIcon::Disabled);
            x += tileSize;
            if (x >= tilesetWidth) {
                x = 0;
                y += tileSize;
            }
        };

        for (int i = 0; i < cSmallIconOnly; ++i) {
            QIcon icon;
            addTile(icon, smallX, smallY, cTileSmall, tilesetWidth, tilesetPix);
            addTile(icon, largeX, largeY, cTileLarge, tilesetWidth, tilesetPix);
            mTable[i] = std::move(icon);
        }
        // these icons only have a small, normal, icon
        for (int i = cSmallIconOnly; i < icons::Count; ++i) {
            QIcon icon;
            icon.addPixmap(
                tilesetPix.copy(smallX, smallY, cTileSmall, cTileSmall));
            smallX += cTileSmall;
            mTable[i] = std::move(icon);
        }
    }

    QIcon const &operator[](icons::Icons icon) const { return mTable[icon]; }

    static IconTable &instance() {
        static IconTable table;
        return table;
    }

private:
    std::array<QIcon, icons::Count> mTable;
};

QToolBar *toolBarWithSize(QWidget *parent, QSize size) {
    QToolBar *result = new QToolBar(parent);
    result->setIconSize(size);
    return result;
}

} // namespace TU

namespace icons {

QToolBar *largeToolBar(QWidget *parent) {
    return TU::toolBarWithSize(parent, largeSize);
}

QToolBar *smallToolBar(QWidget *parent) {
    return TU::toolBarWithSize(parent, smallSize);
}

QIcon get(Icons icon) {
    return TU::IconTable::instance()[icon];
}

QIcon getApp() {
    QIcon result;
    result.addFile(":/img/app-16.png");
    result.addFile(":/img/app-24.png");
    result.addFile(":/img/app-32.png");
    result.addFile(":/img/app-48.png");
    result.addFile(":/img/app-128.png");
    return result;
}

void generate(const ColorTheme &theme) {
    TU::IconTable::instance().generate(theme);
}
} // namespace icons

#undef TU
