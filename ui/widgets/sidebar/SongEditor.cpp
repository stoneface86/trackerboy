
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
    mTempoLabel(new QLabel),
    mPatternSizeSpin(new QSpinBox)
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
    label = new QLabel(tr("Rows"));
    label->setBuddy(mPatternSizeSpin);
    layout->addWidget(label, 2, 0);
    layout->addWidget(mPatternSizeSpin, 2, 1);
    //
    label = new QLabel(tr("Speed"));
    label->setBuddy(mSpeedSpin);
    layout->addWidget(label, 3, 0);
    layout->addWidget(mSpeedSpin, 3, 1);
    //
    layout->addWidget(mSpeedLabel, 4, 0);
    layout->addWidget(mTempoLabel, 4, 1);

    setLayout(layout);

    mRowsPerBeatSpin->setRange(1, 255);
    mRowsPerMeasureSpin->setRange(1, 255);
    mSpeedSpin->setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    mSpeedSpin->setDisplayIntegerBase(16);
    mSpeedSpin->setPrefix("0x");
    mPatternSizeSpin->setRange(1, 256);

    mSpeedLabel->setAlignment(Qt::AlignCenter);
    mTempoLabel->setAlignment(Qt::AlignCenter);

    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    connect(mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), mRowsPerBeatSpin, &QSpinBox::setMaximum);
}

void SongEditor::setModel(SongModel *model) {
    Q_ASSERT(mSongModel == nullptr && model);

    mSongModel = model;

    connect(model, &SongModel::rowsPerBeatChanged, this,
        [this](int rpb) {
            QSignalBlocker blocker(mRowsPerBeatSpin);
            mRowsPerBeatSpin->setValue(rpb);
        });

    connect(model, &SongModel::rowsPerMeasureChanged, this,
        [this](int rpm) {
            QSignalBlocker blocker(mRowsPerMeasureSpin);
            mRowsPerMeasureSpin->setValue(rpm);
        });

    connect(model, &SongModel::speedChanged, this,
        [this](int speed) {
            QSignalBlocker blocker(mSpeedSpin);
            mSpeedSpin->setValue(speed);

            auto speedFloat = trackerboy::speedToFloat((trackerboy::Speed)speed);
            mSpeedLabel->setText(tr("%1 FPR").arg(speedFloat, 0, 'f', 3));
        });

    connect(model, &SongModel::tempoChanged, this,
        [this](float tempo) {
            mTempoLabel->setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
        });

    connect(model, &SongModel::patternSizeChanged, this,
        [this](int rows) {
            QSignalBlocker blocker(mPatternSizeSpin);
            mPatternSizeSpin->setValue(rows);
        });

    model->reload();
    
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), model, &SongModel::setRowsPerBeat);
    connect(mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), model, &SongModel::setRowsPerMeasure);
    connect(mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), model, &SongModel::setSpeed);
    connect(mPatternSizeSpin, qOverload<int>(&QSpinBox::valueChanged), model, &SongModel::setPatternSize);
}
