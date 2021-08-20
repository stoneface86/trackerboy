
#pragma once

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

    Q_OBJECT

public:
    EnvelopeForm(QWidget *parent = nullptr);
    ~EnvelopeForm();

    uint8_t envelope();

    void setEnvelope(uint8_t value);

signals:
    void envelopeChanged(uint8_t value);

private slots:
    void updateEnvelope();

private:
    void setPeriodLabel(int value);

    Q_DISABLE_COPY(EnvelopeForm)

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
