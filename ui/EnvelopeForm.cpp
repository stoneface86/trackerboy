
#include "EnvelopeForm.hpp"


EnvelopeForm::EnvelopeForm(QWidget *parent) :
    mIgnoreChanges(false),
    mEnvelope(0xF0),
    QWidget(parent)
{
    setupUi(this);

    connect(mInitVolumeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EnvelopeForm::updateEnvelope);
    connect(mIncreasingCheckbox, &QCheckBox::stateChanged, this, &EnvelopeForm::updateEnvelope);
    connect(mPeriodSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EnvelopeForm::updateEnvelope);
}

uint8_t EnvelopeForm::envelope() {
    return mEnvelope;
}

void EnvelopeForm::setEnvelope(uint8_t value) {
    mEnvelope = value;
    mIgnoreChanges = true;
    mInitVolumeSpin->setValue(value >> 4);
    mIncreasingCheckbox->setChecked(!!(value & 0x8));
    mPeriodSpin->setValue(value & 0x7);
    mIgnoreChanges = false;
}

void EnvelopeForm::updateEnvelope(int value) {
    (void)value;

    if (!mIgnoreChanges) {
        uint8_t oldValue = mEnvelope;
        mEnvelope = static_cast<uint8_t>(mInitVolumeSpin->value()) << 4 |
            static_cast<uint8_t>(mIncreasingCheckbox->isChecked() ? 0x8 : 0x0) |
            static_cast<uint8_t>(mPeriodSpin->value());
        if (oldValue != mEnvelope) {
            emit envelopeChanged(mEnvelope);
        }
    }
}
