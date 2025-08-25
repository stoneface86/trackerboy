
#pragma once

#include "utils/aliases.hxx"

#include <QList>
#include <QString>

//
// Object containing a QString version of names of objects from the back end.
// Each name has a QString `value`, a boolean `changed` flag that determines if
// the value should be updated in the back end on save, and an integer
// identifer, `id`.
//
// For songs, the id should be in range of 0-255. For instruments and waveforms,
// the id should be in range of 0-63.
//
struct Name {

    bool changed = false;
    u8 id = 0;
    QString value;
};

//
// Alias for a sequential container of Names, using a QList
//
using NameList = QList<Name>;
