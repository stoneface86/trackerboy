
#pragma once

#include "model/SongModel.hpp"

#include "widgets/CustomSpinBox.hpp"
#include "verdigris/wobjectdefs.h"

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

//
// Composite widget for the song settings editor, located in the Sidebar.
//
class SongEditor : public QWidget {

    W_OBJECT(SongEditor)

public:

    explicit SongEditor(SongModel &model, QWidget *parent = nullptr);

private:

    void updatePatternSizeButton(int rows);

    void updateSpeedLabel(int speed);
    void updateTempoLabel(float tempo);

    void refresh();

    SongModel &mSongModel;

    QSpinBox *mRowsPerBeatSpin;
    QSpinBox *mRowsPerMeasureSpin;
    CustomSpinBox *mSpeedSpin;
    QLabel *mSpeedLabel;
    QLabel *mTempoLabel;
    QPushButton *mPatternSizeButton;
};
