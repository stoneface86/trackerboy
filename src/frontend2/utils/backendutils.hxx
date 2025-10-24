
#pragma once

#include "backend.hxx"

#include <QString>

//
// Convert a slice from the backend to a QString
//
extern QString toQString(BSlice slice);

//
// Convert a QString to a nim string. UTF-8 encoding is used.
//
extern BString toNimString(QString const &str);
