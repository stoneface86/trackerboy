
#pragma once

#include "model/SongModel.hpp"

#include "widgets/CustomSpinBox.hpp"
#include "widgets/SpeedLabel.hpp"
#include "widgets/TempoLabel.hpp"

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

//
// Composite widget for the song settings editor, located in the Sidebar.
//
class SongEditor : public QWidget {

    Q_OBJECT

public:

    explicit SongEditor(SongModel &model, QWidget *parent = nullptr);

private:

    SongModel &mSongModel;

    QSpinBox *mRowsPerBeatSpin;
    QSpinBox *mRowsPerMeasureSpin;
    CustomSpinBox *mSpeedSpin;
    SpeedLabel *mSpeedLabel;
    TempoLabel *mTempoLabel;
    QPushButton *mPatternSizeButton;
};
