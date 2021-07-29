
#include "widgets/SpeedLabel.hpp"


SpeedLabel::SpeedLabel(QWidget *parent) :
    QLabel(tr("-- FPR"), parent)
{
}

void SpeedLabel::setSpeed(float speed) {
    setText(tr("%1 FPR").arg(speed, 0, 'f', 3));
}
