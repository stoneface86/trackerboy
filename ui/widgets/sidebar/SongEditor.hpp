
#pragma once

#include "core/model/SongModel.hpp"

#include "widgets/CustomSpinBox.hpp"

#include <QLabel>
#include <QSpinBox>
#include <QWidget>

//
// Composite widget for the song settings editor, located in the Sidebar.
//
class SongEditor : public QWidget {

    Q_OBJECT

public:

    explicit SongEditor(QWidget *parent = nullptr);
    virtual ~SongEditor() = default;

    void setModel(SongModel *model);

private:

    SongModel *mSongModel;

    QSpinBox *mRowsPerBeatSpin;
    QSpinBox *mRowsPerMeasureSpin;
    CustomSpinBox *mSpeedSpin;
    QLabel *mSpeedLabel;
    QLabel *mTempoLabel;
    QSpinBox *mPatternSizeSpin;

};
