
#include "EnvelopeForm.hpp"

#pragma warning(push, 0)
#include "designer/ui_EnvelopeForm.h"
#pragma warning(pop)



EnvelopeForm::EnvelopeForm(QWidget *parent) :
    mUi(new Ui::EnvelopeForm),
    mIgnoreChanges(false),
    mEnvelope(0xF0),
    QWidget(parent)
{
    mUi->setupUi(this);

    connect(mUi->mInitVolumeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EnvelopeForm::updateEnvelope);
    connect(mUi->mIncreasingCheckbox, &QCheckBox::stateChanged, this, &EnvelopeForm::updateEnvelope);
    connect(mUi->mPeriodSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EnvelopeForm::updateEnvelope);
}

uint8_t EnvelopeForm::envelope() {
    return mEnvelope;
}

void EnvelopeForm::setEnvelope(uint8_t value) {
    mEnvelope = value;
    mIgnoreChanges = true;
    mUi->mInitVolumeSpin->setValue(value >> 4);
    mUi->mIncreasingCheckbox->setChecked(!!(value & 0x8));
    mUi->mPeriodSpin->setValue(value & 0x7);
    mIgnoreChanges = false;
}

void EnvelopeForm::updateEnvelope(int value) {
    (void)value;

    if (!mIgnoreChanges) {
        uint8_t oldValue = mEnvelope;
        mEnvelope = static_cast<uint8_t>(mUi->mInitVolumeSpin->value()) << 4 |
            static_cast<uint8_t>(mUi->mIncreasingCheckbox->isChecked() ? 0x8 : 0x0) |
            static_cast<uint8_t>(mUi->mPeriodSpin->value());
        if (oldValue != mEnvelope) {
            emit envelopeChanged(mEnvelope);
        }
    }
}
