
#pragma once

#include <QAction>


// Container for actions used by SongWidget and OrderWidget
struct SongActions {

    SongActions();


    QAction add;
    QAction remove;
    QAction duplicate;
    QAction moveUp;
    QAction moveDown;


};
