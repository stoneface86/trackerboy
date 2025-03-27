
#include "widgets/CustomSpinBox.hxx"

CustomSpinBox::CustomSpinBox(QWidget *parent) :
    QSpinBox(parent),
    mDigits(0)
{
}

void CustomSpinBox::setDigits(int digits) {
    mDigits = digits;
}

QString CustomSpinBox::textFromValue(int value) const {
    return QString::number(value, displayIntegerBase())
            .rightJustified(mDigits, u'0')
            .toUpper();
}
