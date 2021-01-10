
#pragma once

#include "core/model/InstrumentListModel.hpp"
#include "core/model/WaveListModel.hpp"
#include "widgets/EnvelopeForm.hpp"
#include "widgets/PianoWidget.hpp"

#include "trackerboy/ChType.hpp"

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>


class InstrumentEditor : public QDialog {

    Q_OBJECT

public:
    InstrumentEditor(InstrumentListModel &instModel, WaveListModel &waveModel, PianoInput &input, QWidget *parent = nullptr);
    ~InstrumentEditor();

    PianoWidget& piano();

signals:
    // emitted when the wave editor should be shown
    void waveEditorRequested();

private slots:
    void onChannelSelect(int channel);
    void currentInstrumentChanged(int index);

private:

    void updateTimbreCombo(trackerboy::ChType ch);

    InstrumentListModel &mInstrumentModel;
    WaveListModel &mWaveModel;
    bool mIgnoreChanged;
    trackerboy::ChType mLastChannel;

    QVBoxLayout mLayout;
        QHBoxLayout mLayoutSelect;
            QComboBox mChannelCombo;
            QComboBox mInstrumentCombo;
        QHBoxLayout mLayoutGroup;
            QGroupBox mGroupSettings;
                QFormLayout mLayoutSettings;
                    QLabel mTimbreLabel;
                    QComboBox mTimbreCombo;
                    QComboBox mPanningCombo;
                    QSpinBox mDelaySpin;
                    QSpinBox mDurationSpin;
            QGroupBox mGroupFrequency;
                QFormLayout mLayoutFrequency;
                    QSpinBox mTuneSpin;
                    QSpinBox mVibratoSpeedSpin;
                    QSpinBox mVibratoExtentSpin;
                    QSpinBox mVibratoDelaySpin;
            QGroupBox mGroupEnvelope;
                QGridLayout mLayoutEnvelope;
                    EnvelopeForm mEnvelopeForm;
            QGroupBox mGroupWave;
                QVBoxLayout mLayoutWave;
                    QComboBox mWaveCombo;
                    QPushButton mWaveEditButton;
        PianoWidget mPiano;



    
};
