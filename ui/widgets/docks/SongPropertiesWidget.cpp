
#include "widgets/docks/SongPropertiesWidget.hpp"

#include <QFormLayout>

SongPropertiesWidget::SongPropertiesWidget(SongListModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mRowsPerBeatSpin(new QSpinBox()),
    mRowsPerMeasureSpin(new QSpinBox()),
    mSpeedSpin(new QDoubleSpinBox()),
    mTempoEdit(new QLineEdit()),
    mPatternSpin(new QSpinBox()),
    mRowsPerPatternSpin(new QSpinBox()),
    mIgnoreSpeedChanges(false)
{
    setObjectName("SongPropertiesWidget");

    // layout

    auto layout = new QFormLayout();
    layout->addRow(tr("Rows/Beat"), mRowsPerBeatSpin);
    layout->addRow(tr("Rows/measure"), mRowsPerMeasureSpin);

    auto speedLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    speedLayout->addWidget(mSpeedSpin, 1);
    speedLayout->addWidget(mTempoEdit, 1);
    layout->addRow(tr("Speed (Frames/row)"), speedLayout);
    
    
    //layout->addRow(tr("Tempo"), mTempoEdit);
    layout->addRow(tr("Patterns"), mPatternSpin);
    layout->addRow(tr("Rows"), mRowsPerPatternSpin);

    setLayout(layout);

    // settings

    mRowsPerBeatSpin->setRange(1, 255);
    mRowsPerMeasureSpin->setRange(1, 255);
    mSpeedSpin->setDecimals(3);
    mSpeedSpin->setSingleStep(0.125);
    mSpeedSpin->setRange(trackerboy::SPEED_MIN * 0.125, trackerboy::SPEED_MAX * 0.125);
    mTempoEdit->setReadOnly(true);

    onSongChanged(mModel.currentIndex());
    calculateTempo();

    // connections
    connect(&mModel, &SongListModel::currentIndexChanged, this, &SongPropertiesWidget::onSongChanged);
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &mModel, &SongListModel::setRowsPerBeat);
    connect(mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &mModel, &SongListModel::setRowsPerMeasure);
    connect(mSpeedSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SongPropertiesWidget::onSpeedChanged);
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    connect(mSpeedSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, 
        [this](double value) {
            Q_UNUSED(value);
            calculateTempo();
        });
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int value) {
            Q_UNUSED(value);
            calculateTempo();
        });
    
}

SongPropertiesWidget::~SongPropertiesWidget() {
}


void SongPropertiesWidget::onSongChanged(int index) {
    if (index != -1) {

        auto song = mModel.currentSong();

        mRowsPerBeatSpin->setValue(song->rowsPerBeat());
        mRowsPerMeasureSpin->setValue(song->rowsPerMeasure());
        mSpeedSpin->setValue(song->speedF());
        mPatternSpin->setValue(song->orders().size());
        mRowsPerPatternSpin->setValue(song->patterns().rowSize());


    }
}

void SongPropertiesWidget::onSpeedChanged(double speed) {
    if (!mIgnoreSpeedChanges) {
        mIgnoreSpeedChanges = true;

        mModel.setSpeed(speed);
        mSpeedSpin->setValue(mModel.currentSong()->speedF());

        mIgnoreSpeedChanges = false;
    }


}

void SongPropertiesWidget::calculateTempo() {
    auto tempo = mModel.currentSong()->tempo();
    mTempoEdit->setText(QString("%1 BPM").arg(tempo, 0, 'f', 2));
}
