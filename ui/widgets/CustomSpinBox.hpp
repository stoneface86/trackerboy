
#pragma once

#include <QSpinBox>


//
// Custom spinbox widget that zero pads and outputs the string
// in uppercase.
//
class CustomSpinBox : public QSpinBox {

    Q_OBJECT


public:
    CustomSpinBox(QWidget *parent = nullptr);

    void setDigits(int digits);

protected:
    QString textFromValue(int value) const override;

private:

    int mDigits;

};
