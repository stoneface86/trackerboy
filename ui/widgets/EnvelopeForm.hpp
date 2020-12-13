
#pragma once

#include <QCheckBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QWidget>

#include <cstdint>


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
    void updateEnvelope(int value);

private:
    bool mIgnoreChanges;
    uint8_t mEnvelope;

    QFormLayout mLayout;
        QSpinBox mInitVolumeSpin;
        QCheckBox mIncreasingCheckbox;
        QSpinBox mPeriodSpin;

};
