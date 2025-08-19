
#pragma once

#include "utils/aliases.hxx"

#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QWidget>

//
// Composite widget for an envelope editor.
//
class EnvelopeForm final : public QWidget {

    Q_OBJECT

public:
    explicit EnvelopeForm(QWidget *parent = nullptr);

    u8 envelope();

    void setEnvelope(u8 value);

signals:
    void envelopeChanged(u8 value);

private:
    Q_DISABLE_COPY(EnvelopeForm)

    void setPeriodLabel(int value);

    void updateEnvelope();

    bool _ignoreChanges;
    u8 _envelope;

    QSpinBox *_initVolumeSpin;
    QCheckBox *_increasingCheckbox;
    QSpinBox *_periodSpin;
    QLabel *_periodRateLabel;
};
