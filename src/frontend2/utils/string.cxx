
#include "utils/string.hxx"

#include "tbb_nim.h"

#include <QCoreApplication>

QString tempoToString(float const tempo) {
    return QCoreApplication::tr("%1 BPM").arg(tempo, 0, 'f', 2);
}

QString speedToString(float const speed) {
    return QCoreApplication::tr("%1 FPR").arg(speed, 0, 'f', 3);
}

QString toHex(quint8 const byte) {
    QString result(2, Qt::Initialization::Uninitialized);
    auto const hex = bText(byte);
    result[0] = hex.data[0];
    result[1] = hex.data[1];
    return result;
}