
#include "utils/string.hpp"

#include <QCoreApplication>

QString tempoToString(float tempo) {
    return QCoreApplication::tr("%1 BPM").arg(tempo, 0, 'f', 2);
}

QString speedToString(float speed) {
    return QCoreApplication::tr("%1 FPR").arg(speed, 0, 'f', 3);
}
