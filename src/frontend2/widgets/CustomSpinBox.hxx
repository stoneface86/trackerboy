
#pragma once

#include <QSpinBox>

//
// Custom spinbox widget that zero pads and outputs the string
// in uppercase.
//
class CustomSpinBox : public QSpinBox {

    // not needed, no signals, slots or properties defined
    // Q_OBJECT

public:
    explicit CustomSpinBox(QWidget *parent = nullptr);
    virtual ~CustomSpinBox() = default;

    void setDigits(int digits);

protected:
    virtual QString textFromValue(int value) const override;

private:
    Q_DISABLE_COPY(CustomSpinBox)

    int _digits;
};
