
#include "Tileset.hpp"
#include <QPainter>


Tileset::Tileset(QImage image, int width, int height) :
    mTilesetImage(image),
    mTileImage(QSize(width, height), image.format()),
    mTileWidth(width),
    mTileHeight(height)
{

}

Tileset::~Tileset() {

}

QPixmap Tileset::getTile(int row, int column) {
    
    mTileImage.fill(Qt::transparent);
    QPainter painter(&mTileImage);
    painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
    painter.drawImage(0, 0, mTilesetImage, column * mTileWidth, row * mTileHeight, mTileWidth, mTileHeight);
    painter.end();

    return QPixmap::fromImage(mTileImage);
}

QIcon Tileset::getIcon(int index) {
    QIcon result;
    result.addPixmap(getTile(0, index), QIcon::Normal);
    result.addPixmap(getTile(1, index), QIcon::Disabled);
    return result;
}
