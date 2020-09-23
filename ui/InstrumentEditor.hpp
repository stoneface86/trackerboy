
#pragma once

#include <QDialog>

#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"

#include "EnvelopeForm.hpp"

#pragma warning(push, 0)
#include "designer/ui_InstrumentEditor.h"
#pragma warning(pop)

class InstrumentEditor : public QDialog, private Ui::InstrumentEditor {

    Q_OBJECT

public:
    InstrumentEditor(InstrumentListModel &instModel, WaveListModel &waveModel, QWidget &waveEditor, QWidget *parent = nullptr);

    PianoWidget* piano();

private slots:
    void onChannelSelect(int channel);
    void currentInstrumentChanged(int index);

private:

    InstrumentListModel &mInstrumentModel;
    WaveListModel &mWaveModel;
    QWidget &mWaveEditor;

    QGroupBox *mGroupEnvelope = nullptr;
    EnvelopeForm *mEnvelopeForm = nullptr;

    bool mIgnoreChanged;
    trackerboy::ChType mLastChannel;
};
