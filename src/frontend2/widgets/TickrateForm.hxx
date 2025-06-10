#pragma once

#include "backend.hxx"

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QGroupBox>

class TickrateForm : public QGroupBox {

    Q_OBJECT

public:
    explicit TickrateForm(QWidget *parent = nullptr);

    B::Tickrate tickrate() const;
    void setTickrate(B::Tickrate const &tickrate);

signals:
    void tickrateChanged();

private:
    QButtonGroup *mButtons;
    QDoubleSpinBox *mCustomRate;
};
