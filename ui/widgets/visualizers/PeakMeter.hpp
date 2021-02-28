
#pragma once

#include "widgets/visualizers/VolumeMeterAnimation.hpp"

#include <QWidget>

class PeakMeter : public QWidget {

    Q_OBJECT

public:
    PeakMeter(QWidget *parent = nullptr);

public slots:

    void setPeaks(qint16 left, qint16 right);

protected:

    void paintEvent(QPaintEvent *evt) override;

private:
    Q_DISABLE_COPY_MOVE(PeakMeter)

    VolumeMeterAnimation mMeterLeft;
    VolumeMeterAnimation mMeterRight;

};
