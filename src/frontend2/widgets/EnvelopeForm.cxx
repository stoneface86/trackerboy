
#include "widgets/EnvelopeForm.hxx"

#include <QGridLayout>

EnvelopeForm::EnvelopeForm(QWidget *parent) :
    QWidget(parent),
    mIgnoreChanges(false),
    mEnvelope(0xF0),
    mInitVolumeSpin(),
    mIncreasingCheckbox(),
    mPeriodSpin(),
    mPeriodRateLabel()
{
    
    // Layout
    auto layout = new QGridLayout;
    // row 0
    auto initVolumeLabel = new QLabel(tr("Initial volume"));
    layout->addWidget(initVolumeLabel, 0, 0);
    mInitVolumeSpin = new QSpinBox;
    layout->addWidget(mInitVolumeSpin, 0, 1);
    mIncreasingCheckbox = new QCheckBox(tr("Increasing"));
    layout->addWidget(mIncreasingCheckbox, 0, 2);
    // row 1
    auto periodLabel = new QLabel(tr("Period"));
    layout->addWidget(periodLabel, 1, 0);
    mPeriodSpin = new QSpinBox;
    layout->addWidget(mPeriodSpin, 1, 1);
    mPeriodRateLabel = new QLabel;
    layout->addWidget(mPeriodRateLabel, 1, 2);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 3);
    setLayout(layout);

    initVolumeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    periodLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mPeriodRateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // settings
    mInitVolumeSpin->setValue(15);
    mInitVolumeSpin->setRange(0, 15);
    mPeriodSpin->setValue(0);
    mPeriodSpin->setRange(0, 7);
    setPeriodLabel(0);


    connect(mInitVolumeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &EnvelopeForm::updateEnvelope);
    connect(mIncreasingCheckbox, &QCheckBox::checkStateChanged,
            this, &EnvelopeForm::updateEnvelope);
    connect(mPeriodSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            updateEnvelope();
            setPeriodLabel(value);
        });
}

u8 EnvelopeForm::envelope() {
    return mEnvelope;
}

void EnvelopeForm::setEnvelope(u8 value) {
    mEnvelope = value;
    mIgnoreChanges = true;
    mInitVolumeSpin->setValue(value >> 4);
    mIncreasingCheckbox->setChecked(!!(value & 0x8));
    mPeriodSpin->setValue(value & 0x7);
    mIgnoreChanges = false;
}

void EnvelopeForm::updateEnvelope() {

    if (!mIgnoreChanges) {
        u8 e = (u8)mInitVolumeSpin->value() << 4;
        if (mIncreasingCheckbox->isChecked()) {
            e |= 0x8;
        }
        e |= (u8)mPeriodSpin->value();
        if (mEnvelope != e) {
            mEnvelope = e;
            emit envelopeChanged(mEnvelope);
        }
    }
}

void EnvelopeForm::setPeriodLabel(int value) {
    if (value == 0) {
        mPeriodRateLabel->setText(tr("Constant volume"));
    } else {
        mPeriodRateLabel->setText(tr("%1 s/unit").arg(value * (1/64.0f), 0, 'f', 3));
    }
}
