
#include "widgets/visualizers/PeakMeter.hpp"

#include <QPaintEvent>
#include <QPainter>


PeakMeter::PeakMeter(QWidget *parent) :
    QWidget(parent)
{
    connect(&mMeterLeft, &VolumeMeterAnimation::redraw, this, qOverload<>(&PeakMeter::update));
    connect(&mMeterRight, &VolumeMeterAnimation::redraw, this, qOverload<>(&PeakMeter::update));
}


void PeakMeter::setPeaks(qint16 left, qint16 right) {

    mMeterLeft.setTarget(left);
    mMeterRight.setTarget(right);

}

void PeakMeter::paintEvent(QPaintEvent *evt) {
    Q_UNUSED(evt)

    QPainter painter(this);

    int const w = width();
    int const center = w / 2;

    painter.fillRect(QRectF(center, 0.0, mMeterLeft.meterWidth(center), 16.0), Qt::black);
    painter.fillRect(QRectF(center, 0.0, -mMeterRight.meterWidth(center), 16.0), Qt::black);
}
