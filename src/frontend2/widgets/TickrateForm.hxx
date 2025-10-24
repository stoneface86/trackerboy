#pragma once

#include "backend.hxx"

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QGroupBox>

class TickrateForm : public QGroupBox {

    Q_OBJECT

public:
    explicit TickrateForm(QWidget *parent = nullptr);

    BTickrate tickrate() const;
    void setTickrate(BTickrate const &tickrate);

signals:
    void tickrateChanged();

private:
    QButtonGroup *_buttons;
    QDoubleSpinBox *_customRate;
};
