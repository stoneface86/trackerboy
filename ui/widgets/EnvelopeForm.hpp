
#pragma once

#include <QWidget>

#include <cstdint>

namespace Ui {
    class EnvelopeForm;
}

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
    Ui::EnvelopeForm *mUi;
    bool mIgnoreChanges;
    uint8_t mEnvelope;

};
