
#pragma once

#include <QAction>

//
// POD struct of QActions for the waveform/instrument table
//
struct TableActions {

    QAction *add = nullptr;
    QAction *remove = nullptr;
    QAction *duplicate = nullptr;
    QAction *importFile = nullptr;
    QAction *exportFile = nullptr;
    QAction *edit = nullptr;

};
