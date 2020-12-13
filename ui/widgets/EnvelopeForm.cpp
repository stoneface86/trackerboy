
#include "EnvelopeForm.hpp"

EnvelopeForm::EnvelopeForm(QWidget *parent) :
    QWidget(parent),
    mIgnoreChanges(false),
    mEnvelope(0xF0),
    mLayout(),
    mInitVolumeSpin(),
    mIncreasingCheckbox(tr("Increasing")),
    mPeriodSpin()
{
    
    // Layout
    mLayout.addRow(tr("Initial volume"), &mInitVolumeSpin);
    mLayout.addRow(QString(), &mIncreasingCheckbox);
    mLayout.addRow(tr("Period"), &mPeriodSpin);
    setLayout(&mLayout);

    // settings
    mInitVolumeSpin.setValue(15);
    mInitVolumeSpin.setRange(0, 15);
    mPeriodSpin.setValue(0);
    mPeriodSpin.setRange(0, 7);


    connect(&mInitVolumeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EnvelopeForm::updateEnvelope);
    connect(&mIncreasingCheckbox, &QCheckBox::stateChanged, this, &EnvelopeForm::updateEnvelope);
    connect(&mPeriodSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EnvelopeForm::updateEnvelope);
}

EnvelopeForm::~EnvelopeForm() {
}

uint8_t EnvelopeForm::envelope() {
    return mEnvelope;
}

void EnvelopeForm::setEnvelope(uint8_t value) {
    mEnvelope = value;
    mIgnoreChanges = true;
    mInitVolumeSpin.setValue(value >> 4);
    mIncreasingCheckbox.setChecked(!!(value & 0x8));
    mPeriodSpin.setValue(value & 0x7);
    mIgnoreChanges = false;
}

void EnvelopeForm::updateEnvelope(int value) {
    (void)value;

    if (!mIgnoreChanges) {
        uint8_t oldValue = mEnvelope;
        mEnvelope = static_cast<uint8_t>(mInitVolumeSpin.value()) << 4 |
            static_cast<uint8_t>(mIncreasingCheckbox.isChecked() ? 0x8 : 0x0) |
            static_cast<uint8_t>(mPeriodSpin.value());
        if (oldValue != mEnvelope) {
            emit envelopeChanged(mEnvelope);
        }
    }
}
