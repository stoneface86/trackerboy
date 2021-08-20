
#pragma once

#include <QLabel>

class SpeedLabel : public QLabel {

    Q_OBJECT

public:

    explicit SpeedLabel(QWidget *parent = nullptr);

    void setSpeed(float speed);

};
