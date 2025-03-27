
#pragma once

#include "backend.hxx"

#include <QByteArray>
#include <QString>

struct Utf8Slice {
    QByteArray data;
    B::Slice slice;
};

extern QString toString(B::Slice slice);
extern Utf8Slice toUtf8(QString const& str);
