
#include "widgets/CustomSpinBox.hpp"


CustomSpinBox::CustomSpinBox(QWidget *parent) :
    QSpinBox(parent),
    mDigits(0)
{
}

void CustomSpinBox::setDigits(int digits) {
    mDigits = digits;
}

QString CustomSpinBox::textFromValue(int value) const {
    return QStringLiteral("%1").arg(value, mDigits, displayIntegerBase(), QChar('0')).toUpper();
}
