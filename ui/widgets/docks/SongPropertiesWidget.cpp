
#include "widgets/docks/SongPropertiesWidget.hpp"

#include <QFormLayout>

SongPropertiesWidget::SongPropertiesWidget(SongListModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mRowsPerBeatSpin(new QSpinBox()),
    mRowsPerMeasureSpin(new QSpinBox()),
    mTempoRadio(new QRadioButton(tr("Tempo"))),
    mSpeedRadio(new QRadioButton(tr("Speed"))),
    mTempoSpin(new QSpinBox()),
    mSpeedSpin(new QDoubleSpinBox()),
    mPatternSpin(new QSpinBox()),
    mRowsPerPatternSpin(new QSpinBox())
{
    setObjectName("SongPropertiesWidget");

    // layout

    auto layout = new QFormLayout();
    layout->addRow(tr("Rows/Beat"), mRowsPerBeatSpin);
    layout->addRow(tr("Rows/measure"), mRowsPerMeasureSpin);

    auto modeLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    modeLayout->addWidget(mTempoRadio);
    modeLayout->addWidget(mSpeedRadio);
    

    layout->addRow(tr("Mode"), modeLayout);

    layout->addRow(tr("Tempo (BPM)"), mTempoSpin);
    layout->addRow(tr("Speed (Frames/row)"), mSpeedSpin);
    layout->addRow(tr("Patterns"), mPatternSpin);
    layout->addRow(tr("Rows"), mRowsPerPatternSpin);

    setLayout(layout);

    // settings

    mRowsPerBeatSpin->setRange(1, 255);
    mRowsPerMeasureSpin->setRange(1, 255);
    mTempoRadio->setChecked(true);
    mTempoSpin->setRange(0, INT16_MAX);
    mTempoSpin->setValue(150);
    mSpeedSpin->setDecimals(3);
    mSpeedSpin->setSingleStep(0.125);
    mSpeedSpin->setRange(1.0, 30.875);
    mSpeedSpin->setValue(6.0);

    // connections
    connect(&mModel, &SongListModel::currentIndexChanged, this, &SongPropertiesWidget::onSongChanged);

}

SongPropertiesWidget::~SongPropertiesWidget() {
}


void SongPropertiesWidget::onSongChanged(int index) {
    if (index != -1) {

        auto song = mModel.currentSong();

        mRowsPerBeatSpin->setValue(song->rowsPerBeat());
        bool isSpeedMode = song->mode() == trackerboy::Song::Mode::speed;

        mSpeedRadio->setChecked(isSpeedMode);
        mSpeedSpin->setEnabled(isSpeedMode);
        mTempoSpin->setEnabled(!isSpeedMode);

        mTempoSpin->setValue(song->tempo());
        mSpeedSpin->setValue(song->speed());
        mPatternSpin->setValue(song->orders().size());
        mRowsPerPatternSpin->setValue(song->patterns().rowSize());


    }
}
