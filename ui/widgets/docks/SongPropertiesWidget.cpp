
#include "widgets/docks/SongPropertiesWidget.hpp"

#include <QFormLayout>
#include <QGridLayout>

#include <algorithm>
#include <cmath>

SongPropertiesWidget::SongPropertiesWidget(SongListModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mRowsPerBeatSpin(new QSpinBox()),
    mRowsPerMeasureSpin(new QSpinBox()),
    mSpeedSpin(new QSpinBox()),
    mTempoSpin(new QSpinBox()),
    mTempoCalcButton(new QPushButton(tr("Calculate speed"))),
    mTempoActualEdit(new QLineEdit()),
    mPatternSpin(new QSpinBox()),
    mRowsPerPatternSpin(new QSpinBox())
{
    setObjectName("SongPropertiesWidget");

    // layout

    auto layout = new QFormLayout();
    layout->addRow(tr("Rows per beat"), mRowsPerBeatSpin);
    layout->addRow(tr("Rows per measure"), mRowsPerMeasureSpin);

    auto speedLayout = new QGridLayout();
    speedLayout->addWidget(mSpeedSpin, 0, 0);
    speedLayout->addWidget(mTempoActualEdit, 0, 1);
    speedLayout->addWidget(mTempoSpin, 1, 0);
    speedLayout->addWidget(mTempoCalcButton, 1, 1);
    speedLayout->setColumnStretch(0, 1);
    speedLayout->setColumnStretch(1, 1);
    layout->addRow(tr("Speed"), speedLayout);
    
    layout->addRow(tr("Patterns"), mPatternSpin);
    layout->addRow(tr("Rows"), mRowsPerPatternSpin);

    setLayout(layout);

    // settings

    mRowsPerBeatSpin->setRange(1, 255);
    mRowsPerMeasureSpin->setRange(1, 255);
    mSpeedSpin->setRange(trackerboy::SPEED_MIN, trackerboy::SPEED_MAX);
    mSpeedSpin->setDisplayIntegerBase(16);
    mSpeedSpin->setPrefix("0x");
    mTempoSpin->setRange(1, 10000);
    mTempoSpin->setValue(150);
    mTempoSpin->setSuffix(" BPM");
    mTempoActualEdit->setReadOnly(true);
    mPatternSpin->setRange(1, trackerboy::Song::MAX_ORDERS);
    mRowsPerPatternSpin->setRange(1, 256);

    onSongChanged(mModel.currentIndex());
    calculateActualTempo();

    // connections
    connect(&mModel, &SongListModel::currentIndexChanged, this, &SongPropertiesWidget::onSongChanged);
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), &mModel, &SongListModel::setRowsPerBeat);
    connect(mRowsPerMeasureSpin, qOverload<int>(&QSpinBox::valueChanged), &mModel, &SongListModel::setRowsPerMeasure);
    connect(mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), &mModel, &SongListModel::setSpeed);
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), mRowsPerMeasureSpin, &QSpinBox::setMinimum);
    connect(mSpeedSpin, qOverload<int>(&QSpinBox::valueChanged), this, &SongPropertiesWidget::calculateActualTempo);
    connect(mRowsPerBeatSpin, qOverload<int>(&QSpinBox::valueChanged), this, &SongPropertiesWidget::calculateActualTempo);
    connect(mTempoCalcButton, &QPushButton::clicked, this, &SongPropertiesWidget::calculateTempo);
    connect(mPatternSpin, qOverload<int>(&QSpinBox::valueChanged), &mModel, &SongListModel::setPatterns);
    connect(mRowsPerPatternSpin, qOverload<int>(&QSpinBox::valueChanged), &mModel, &SongListModel::setRowsPerPattern);

    auto orderModel = mModel.orderModel();
    connect(orderModel, &OrderModel::rowsInserted, this, &SongPropertiesWidget::updatePatternSpin);
    connect(orderModel, &OrderModel::rowsRemoved, this, &SongPropertiesWidget::updatePatternSpin);

}

SongPropertiesWidget::~SongPropertiesWidget() {
}


void SongPropertiesWidget::onSongChanged(int index) {
    if (index != -1) {

        auto song = mModel.currentSong();

        mRowsPerBeatSpin->setValue(song->rowsPerBeat());
        mRowsPerMeasureSpin->setValue(song->rowsPerMeasure());
        mSpeedSpin->setValue(song->speed());
        mPatternSpin->setValue(song->orders().size());
        mRowsPerPatternSpin->setValue(song->patterns().rowSize());


    }
}

void SongPropertiesWidget::calculateTempo() {
    // TODO: get the framerate set in the module
    float speed = (trackerboy::Gbs::FRAMERATE_GB * 60.0f) / (mTempoSpin->value() * mRowsPerBeatSpin->value());
    // convert to fixed point
    int speedFixed = std::clamp(static_cast<int>(roundf(speed * 16.0f)), (int)trackerboy::SPEED_MIN, (int)trackerboy::SPEED_MAX);
    mSpeedSpin->setValue(speedFixed);

}

void SongPropertiesWidget::calculateActualTempo(int value) {
    Q_UNUSED(value);

    auto tempo = mModel.currentSong()->tempo();
    mTempoActualEdit->setText(QString("%1 BPM").arg(tempo, 0, 'f', 2));
}

void SongPropertiesWidget::updatePatternSpin(const QModelIndex &parent, int first, int last) {
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

    mPatternSpin->setValue(mModel.currentSong()->orders().size());
}
