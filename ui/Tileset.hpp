
#pragma once

#include <QIcon>
#include <QImage>
#include <QPixmap>

//
// Utility class for getting tiles or individual QPixmaps from a tileset
//
class Tileset {

public:

    Tileset(QImage image, int tileWidth, int tileHeight);
    ~Tileset();


    QPixmap getTile(int row, int column);

    QIcon getIcon(int index);

private:

    QImage mTilesetImage;
    QImage mTileImage;

    int const mTileWidth;
    int const mTileHeight;


};
