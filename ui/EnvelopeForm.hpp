
#pragma once

#include <cstdint>

#include <QWidget>

#pragma warning(push, 0)
#include "designer/ui_EnvelopeForm.h"
#pragma warning(pop)


class EnvelopeForm : public QWidget, private Ui::EnvelopeForm {

    Q_OBJECT

public:
    EnvelopeForm(QWidget *parent = nullptr);

    uint8_t envelope();

    void setEnvelope(uint8_t value);

signals:
    void envelopeChanged(uint8_t value);

private slots:
    void updateEnvelope(int value);

private:
    bool mIgnoreChanges;
    uint8_t mEnvelope;

};
