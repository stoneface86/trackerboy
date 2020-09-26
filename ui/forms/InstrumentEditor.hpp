
#pragma once

#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"
#include "widgets/EnvelopeForm.hpp"
#include "widgets/PianoWidget.hpp"

#include <QDialog>
#include <QGroupBox>
#include <QWidget>

#include <memory>

namespace Ui {
class InstrumentEditor;
}

class InstrumentEditor : public QDialog {

    Q_OBJECT

public:
    InstrumentEditor(InstrumentListModel &instModel, WaveListModel &waveModel, QWidget &waveEditor, QWidget *parent = nullptr);
    

    PianoWidget* piano();

private slots:
    void onChannelSelect(int channel);
    void currentInstrumentChanged(int index);

private:

    std::unique_ptr<Ui::InstrumentEditor> mUi;

    InstrumentListModel &mInstrumentModel;
    WaveListModel &mWaveModel;
    QWidget &mWaveEditor;

    QGroupBox *mGroupEnvelope = nullptr;
    EnvelopeForm *mEnvelopeForm = nullptr;

    bool mIgnoreChanged;
    trackerboy::ChType mLastChannel;
};
