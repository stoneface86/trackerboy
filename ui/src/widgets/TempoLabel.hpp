
#pragma once

#include <QLabel>

class TempoLabel : public QLabel {

    Q_OBJECT

public:

    explicit TempoLabel(QWidget *parent = nullptr);

    void setTempo(float tempo);

};
