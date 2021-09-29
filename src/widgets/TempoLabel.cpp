
#include "widgets/TempoLabel.hpp"


TempoLabel::TempoLabel(QWidget *parent) :
    QLabel(tr("-- BPM"), parent)
{
}

void TempoLabel::setTempo(float tempo) {
    setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
}
