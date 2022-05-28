
#pragma once

#include "verdigris/wobjectdefs.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QWidget>

#include <cstdint>

//
// Composite widget for an envelope editor.
//
class EnvelopeForm : public QWidget {

    W_OBJECT(EnvelopeForm)

public:
    EnvelopeForm(QWidget *parent = nullptr);
    ~EnvelopeForm();

    uint8_t envelope();

    void setEnvelope(uint8_t value);

//signals:
    void envelopeChanged(uint8_t value) W_SIGNAL(envelopeChanged, value)

private:
    Q_DISABLE_COPY(EnvelopeForm)

    void setPeriodLabel(int value);
    
    void updateEnvelope();

    bool mIgnoreChanges;
    uint8_t mEnvelope;

    QGridLayout mLayout;
        // row 0
        QLabel mInitVolumeLabel;
        QSpinBox mInitVolumeSpin;
        QCheckBox mIncreasingCheckbox;
        // row 1
        QLabel mPeriodLabel;
        QSpinBox mPeriodSpin;
        QLabel mPeriodRateLabel;

};
