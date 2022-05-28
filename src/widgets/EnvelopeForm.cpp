
#include "EnvelopeForm.hpp"
#include "verdigris/wobjectimpl.h"

W_OBJECT_IMPL(EnvelopeForm)

EnvelopeForm::EnvelopeForm(QWidget *parent) :
    QWidget(parent),
    mIgnoreChanges(false),
    mEnvelope(0xF0),
    mLayout(),
    mInitVolumeLabel(tr("Initial volume")),
    mInitVolumeSpin(),
    mIncreasingCheckbox(tr("Increasing")),
    mPeriodLabel(tr("Period")),
    mPeriodSpin(),
    mPeriodRateLabel()
{
    
    // Layout
    mLayout.addWidget(&mInitVolumeLabel, 0, 0);
    mLayout.addWidget(&mInitVolumeSpin, 0, 1);
    mLayout.addWidget(&mIncreasingCheckbox, 0, 2);
    mLayout.addWidget(&mPeriodLabel, 1, 0);
    mLayout.addWidget(&mPeriodSpin, 1, 1);
    mLayout.addWidget(&mPeriodRateLabel, 1, 2);
    mLayout.addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 3);
    setLayout(&mLayout);

    mInitVolumeLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mPeriodLabel.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mPeriodRateLabel.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // settings
    mInitVolumeSpin.setValue(15);
    mInitVolumeSpin.setRange(0, 15);
    mPeriodSpin.setValue(0);
    mPeriodSpin.setRange(0, 7);
    setPeriodLabel(0);


    connect(&mInitVolumeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EnvelopeForm::updateEnvelope);
    connect(&mIncreasingCheckbox, &QCheckBox::stateChanged, this, &EnvelopeForm::updateEnvelope);
    connect(&mPeriodSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            updateEnvelope();
            setPeriodLabel(value);
        });
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

void EnvelopeForm::updateEnvelope() {

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

void EnvelopeForm::setPeriodLabel(int value) {
    if (value == 0) {
        mPeriodRateLabel.setText(tr("Constant volume"));
    } else {
        mPeriodRateLabel.setText(tr("%1 s/unit").arg(value * (1/64.0f), 0, 'f', 3));
    }
}
