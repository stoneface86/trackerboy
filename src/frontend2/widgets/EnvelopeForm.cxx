
#include "widgets/EnvelopeForm.hxx"

#include <QGridLayout>

EnvelopeForm::EnvelopeForm(QWidget *parent)
    : QWidget(parent)
    , _ignoreChanges(false)
    , _envelope(0xF0)
    , _initVolumeSpin(nullptr)
    , _increasingCheckbox(nullptr)
    , _periodSpin(nullptr)
    , _periodRateLabel(nullptr) {

    // Layout
    auto layout = new QGridLayout;
    // row 0
    auto initVolumeLabel = new QLabel(tr("Initial volume"));
    layout->addWidget(initVolumeLabel, 0, 0);
    _initVolumeSpin = new QSpinBox;
    layout->addWidget(_initVolumeSpin, 0, 1);
    _increasingCheckbox = new QCheckBox(tr("Increasing"));
    layout->addWidget(_increasingCheckbox, 0, 2);
    // row 1
    auto periodLabel = new QLabel(tr("Period"));
    layout->addWidget(periodLabel, 1, 0);
    _periodSpin = new QSpinBox;
    layout->addWidget(_periodSpin, 1, 1);
    _periodRateLabel = new QLabel;
    layout->addWidget(_periodRateLabel, 1, 2);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 3);
    setLayout(layout);

    initVolumeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    periodLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _periodRateLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // settings
    _initVolumeSpin->setValue(15);
    _initVolumeSpin->setRange(0, 15);
    _periodSpin->setValue(0);
    _periodSpin->setRange(0, 7);
    setPeriodLabel(0);

    connect(_initVolumeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &EnvelopeForm::updateEnvelope);
    connect(_increasingCheckbox, &QCheckBox::checkStateChanged, this,
            &EnvelopeForm::updateEnvelope);
    connect(_periodSpin, qOverload<int>(&QSpinBox::valueChanged), this,
            [this](int value) {
                updateEnvelope();
                setPeriodLabel(value);
            });
}

u8 EnvelopeForm::envelope() {
    return _envelope;
}

void EnvelopeForm::setEnvelope(u8 value) {
    _envelope = value;
    _ignoreChanges = true;
    _initVolumeSpin->setValue(value >> 4);
    _increasingCheckbox->setChecked(!!(value & 0x8));
    _periodSpin->setValue(value & 0x7);
    _ignoreChanges = false;
}

void EnvelopeForm::updateEnvelope() {

    if (!_ignoreChanges) {
        u8 e = (u8)_initVolumeSpin->value() << 4;
        if (_increasingCheckbox->isChecked()) {
            e |= 0x8;
        }
        e |= (u8)_periodSpin->value();
        if (_envelope != e) {
            _envelope = e;
            emit envelopeChanged(_envelope);
        }
    }
}

void EnvelopeForm::setPeriodLabel(int value) {
    if (value == 0) {
        _periodRateLabel->setText(tr("Constant volume"));
    } else {
        _periodRateLabel->setText(
            tr("%1 s/unit").arg(value * (1 / 64.0f), 0, 'f', 3));
    }
}
