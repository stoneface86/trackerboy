
#include "widgets/CustomSpinBox.hxx"

CustomSpinBox::CustomSpinBox(QWidget *parent)
    : QSpinBox(parent)
    , _digits(0) {}

void CustomSpinBox::setDigits(int digits) {
    _digits = digits;
}

QString CustomSpinBox::textFromValue(int value) const {
    return QString::number(value, displayIntegerBase())
        .rightJustified(_digits, u'0')
        .toUpper();
}
