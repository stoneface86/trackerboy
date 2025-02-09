
#pragma once

#include <QSpinBox>


//
// Custom spinbox widget that zero pads and outputs the string
// in uppercase.
//
class CustomSpinBox : public QSpinBox {

    // not needed, no signals, slots or properties defined
    //Q_OBJECT

public:
    CustomSpinBox(QWidget *parent = nullptr);

    void setDigits(int digits);

protected:
    QString textFromValue(int value) const override;

private:
    Q_DISABLE_COPY(CustomSpinBox)

    int mDigits;

};
