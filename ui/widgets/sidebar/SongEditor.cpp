
#include "widgets/sidebar/SongEditor.hpp"

#include <QGridLayout>

#include <cmath>

SongEditor::SongEditor(QWidget *parent) :
    QWidget(parent),
    mSongModel(nullptr),
    mRowsPerBeatSpin(new QSpinBox),
    mRowsPerMeasureSpin(new QSpinBox),
    mSpeedSpin(new CustomSpinBox),
    mSpeedLabel(new QLabel),
    mTempoSpin(new QSpinBox),
    mTempoLabel(new QLabel),
    mPatternSizeSpin(new QSpinBox),
    mSpeedLock(false)
{

    auto layout = new QGridLayout;
    auto label = new QLabel(tr("Rows/Beat"));
    label->setBuddy(mRowsPerBeatSpin);
    layout->addWidget(label, 0, 0);
    layout->addWidget(mRowsPerBeatSpin, 0, 1);
    //
    label = new QLabel(tr("Rows/measure"));
    label->setBuddy(mRowsPerMeasureSpin);
    layout->addWidget(label, 1, 0);
    layout->addWidget(mRowsPerMeasureSpin, 1, 1);
    //
    label = new QLabel(tr("Speed"));
    label->setBuddy(mSpeedSpin);
    layout->addWidget(label, 2, 0);
    layout->addWidget(mSpeedSpin, 2, 1);
    layout->addWidget(mSpeedLabel, 2, 2);
    //
    label = new QLabel(tr("Tempo"));
    label->setBuddy(mTempoSpin);
    layout->addWidget(label, 3, 0);
    layout->addWidget(mTempoSpin, 3, 1);
    layout->addWidget(mTempoLabel, 3, 2);
    //
    label = new QLabel(tr("Rows"));
    label->setBuddy(mPatternSizeSpin);
    layout->addWidget(label, 4, 0);
    layout->addWidget(mPatternSizeSpin);

    setLayout(layout);

    mRowsPerBeatSpin->setRange(1, 255);
    mRowsPerMeasureSpin->setRange(1, 255);
    mSpeedSpin->setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    mSpeedSpin->setDisplayIntegerBase(16);
    mSpeedSpin->setPrefix("0x");
    mTempoSpin->setRange(1, 10000);
    mTempoSpin->setValue(150);
    mTempoSpin->setSuffix(" BPM");
    mPatternSizeSpin->setRange(1, 256);

    mSpeedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mTempoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    connect(mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), mRowsPerBeatSpin, &QSpinBox::setMaximum);
}

void SongEditor::setModel(SongModel *model) {
    Q_ASSERT(mSongModel == nullptr && model);

    mSongModel = model;

    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this]() {
            // human-readable speed value
            float speed = trackerboy::speedToFloat((trackerboy::Speed)mSpeedSpin->value());
            float tempo = calcActualTempo(speed);
            setTempoLabel(tempo);
            mSpeedLock = true;
            mTempoSpin->setValue((int)roundf(tempo));
            mSpeedLock = false;
        });
    connect(mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            auto speed = trackerboy::speedToFloat((trackerboy::Speed)value);
            auto tempo = calcActualTempo(speed);
            setTempoLabel(tempo);
            mSpeedLabel->setText(tr("%1 FPR").arg(speed, 0, 'f', 3));
            
            // determine the actual tempo from the speed
            if (!mSpeedLock) {
                mSpeedLock = true;
                mTempoSpin->setValue((int)roundf(tempo));
                mSpeedLock = false;
            }

        });
    connect(mTempoSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            // if (!mSpeedLock) {
            //     mSpeedLock = true;
            //     // convert tempo to speed and set in the speed spin
            //     // speed = (framerate * 60) / (tempo * rpb)
            //     float speed = (mModule.data().framerate() * 60.0f) / (value * mRowsPerBeatSpin.value());
            //     mSpeedSpin.setValue((int)roundf(speed * (1 << trackerboy::SPEED_FRACTION_BITS)));
            //     mSpeedLock = false;
            // }
        });
    

    mRowsPerBeatSpin->setValue(model->rowsPerBeat());
    mRowsPerMeasureSpin->setValue(model->rowsPerMeasure());
    mSpeedSpin->setValue(model->speed());
    mPatternSizeSpin->setValue(model->patternSize());
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), model, &SongModel::setRowsPerBeat);
    connect(mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), model, &SongModel::setRowsPerMeasure);
    connect(mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), model, &SongModel::setSpeed);
    connect(mPatternSizeSpin, qOverload<int>(&QSpinBox::valueChanged), model, &SongModel::setPatternSize);


}

void SongEditor::setTempoLabel(float tempo) {
    mTempoLabel->setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
}

float SongEditor::calcActualTempo(float speed) {
    // actual tempo value
    //tempo = (framerate * 60) / (speed * rpb)
    //convert fixed point to floating point
    //return (mDocument.mod().framerate() * 60.0f) / (speed * mRowsPerBeatSpin.value());
    return 0.0f;
}

