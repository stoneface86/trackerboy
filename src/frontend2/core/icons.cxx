
#include "core/icons.hxx"
#include "utils/aliases.hxx"

#include <array>
#include <utility>

#define TU IconLocatorTU
namespace TU {

using namespace icons;

// template <typename... Args>
// constexpr auto makeSet(Args &&...args) {
//     using Array = std::array<i8, sizeof...(args)>;
//     return Array{static_cast<i8>(args)...};
// }

static constexpr auto cLargeStart = New;
static constexpr auto cLargeEnd = Down;
static constexpr auto cDisableStart = Undo;
static constexpr auto cDisableEnd = Edit;

static void addTile(QIcon &icon, int &x, int &y, int const tileSize,
                    int const tilesetWidth, QPixmap const &tileset,
                    QIcon::Mode mode = QIcon::Normal) {
    icon.addPixmap(tileset.copy(x, y, tileSize, tileSize), mode);
    x += tileSize;
    if (x >= tilesetWidth) {
        x = 0;
        y += tileSize;
    }
}

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
        auto const tilesetWidth = tilesetPix.width();

        int x = 0;
        int y = 0;

        // small, normal icons
        for (int i = 0; i < Count; ++i) {
            QIcon icon;
            addTile(icon, x, y, cTileSmall, tilesetWidth, tilesetPix);
            // overwrite existing icons in the table if present
            mTable[i] = std::move(icon);
        }
        // small, disabled icons
        for (int i = cDisableStart; i <= cDisableEnd; ++i) {
            addTile(mTable[i], x, y, cTileSmall, tilesetWidth, tilesetPix,
                    QIcon::Disabled);
        }
        x = 0;
        y += cTileSmall;
        // large, normal icons
        for (int i = cLargeStart; i <= cLargeEnd; ++i) {
            addTile(mTable[i], x, y, cTileLarge, tilesetWidth, tilesetPix);
        }
        // large, disabled icons
        for (int i = cDisableStart; i <= cLargeEnd; ++i) {
            addTile(mTable[i], x, y, cTileLarge, tilesetWidth, tilesetPix,
                    QIcon::Disabled);
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
