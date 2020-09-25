
#pragma once

#include <cstdint>
#include <memory>

#include <QWidget>



namespace Ui {
    class EnvelopeForm;
}

class EnvelopeForm : public QWidget {

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
    std::unique_ptr<Ui::EnvelopeForm> mUi;
    bool mIgnoreChanges;
    uint8_t mEnvelope;

};
