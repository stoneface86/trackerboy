
#pragma once

#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"
#include "widgets/EnvelopeForm.hpp"
#include "widgets/PianoWidget.hpp"

#include <QDialog>
#include <QGroupBox>
#include <QWidget>

namespace Ui {
class InstrumentEditor;
}

class InstrumentEditor : public QDialog {

    Q_OBJECT

public:
    InstrumentEditor(InstrumentListModel &instModel, WaveListModel &waveModel, QWidget &waveEditor, QWidget *parent = nullptr);
    ~InstrumentEditor();

    PianoWidget* piano();

private slots:
    void onChannelSelect(int channel);
    void currentInstrumentChanged(int index);

private:

    Ui::InstrumentEditor *mUi;

    InstrumentListModel &mInstrumentModel;
    WaveListModel &mWaveModel;
    QWidget &mWaveEditor;

    bool mIgnoreChanged;
    trackerboy::ChType mLastChannel;
};
