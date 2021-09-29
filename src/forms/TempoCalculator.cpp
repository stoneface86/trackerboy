
#include "forms/TempoCalculator.hpp"

#include "core/misc/connectutils.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

#include <cmath>

TempoCalculator::TempoCalculator(SongModel &model, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mModel(model),
    mTempoLabel(nullptr),
    mTempoSpin(nullptr),
    mSpeedLabel(nullptr),
    mSpeedSpin(nullptr)
{
    setWindowTitle(tr("Tempo calculator"));

    auto layout = new QGridLayout;

    QLabel *label;
    label = new QLabel(tr("Tempo"));
    mTempoSpin = new QSpinBox;
    mTempoLabel = new TempoLabel;
    label->setBuddy(mTempoSpin);
    layout->addWidget(label, 0, 0);
    layout->addWidget(mTempoSpin, 0, 1);
    layout->addWidget(mTempoLabel, 0, 2);

    label = new QLabel(tr("Speed"));
    mSpeedSpin = new CustomSpinBox;
    mSpeedLabel = new SpeedLabel;
    label->setBuddy(mSpeedSpin);
    layout->addWidget(label, 1, 0);
    layout->addWidget(mSpeedSpin, 1, 1);
    layout->addWidget(mSpeedLabel, 1, 2);

    auto setButton = new QPushButton(tr("Set song speed"));
    setButton->setDefault(true);
    auto closeButton = new QPushButton(tr("Close"));
    layout->addWidget(setButton, 2, 1);
    layout->addWidget(closeButton, 2, 2);

    layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setLayout(layout);

    mTempoLabel->setAlignment(Qt::AlignCenter);
    mSpeedLabel->setAlignment(Qt::AlignCenter);

    mSpeedSpin->setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    mSpeedSpin->setDisplayIntegerBase(16);
    mSpeedSpin->setPrefix(QStringLiteral("0x"));

    mTempoSpin->setRange(1, 10000);
    mTempoSpin->setSuffix(tr(" BPM"));

    connect(mSpeedSpin, qOverload<int>(&CustomSpinBox::valueChanged), this,
        [this](int value) {
            auto tempo = calculate((trackerboy::Speed)value);
            QSignalBlocker blocker(mTempoSpin);
            mTempoSpin->setValue((int)roundf(tempo));
        });

    connect(mTempoSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            auto speedEst = mModel.estimateSpeed(value);
            calculate(speedEst);

            QSignalBlocker blocker(mSpeedSpin);
            mSpeedSpin->setValue(speedEst);
        });
    
    mSpeedSpin->setValue(0x60);

    lazyconnect(closeButton, clicked, this, close);
    connect(setButton, &QPushButton::clicked, this,
        [this]() {
            mModel.setSpeed(mSpeedSpin->value());
        });

}

float TempoCalculator::calculate(trackerboy::Speed speed) {
    auto speedFloat = trackerboy::speedToFloat(speed);
    auto tempo = trackerboy::speedToTempo(speedFloat, mModel.rowsPerBeat());
    mSpeedLabel->setSpeed(speedFloat);
    mTempoLabel->setTempo(tempo);
    return tempo;
}
