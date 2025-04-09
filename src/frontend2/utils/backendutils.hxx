
#pragma once

#include "backend.hxx"

#include <QString>


//
// Convert a slice from the backend to a QString
//
extern QString toQString(B::Slice slice);

//
// Convert a QString to a nim string. UTF-8 encoding is used.
//
extern B::String toNimString(QString const& str);
