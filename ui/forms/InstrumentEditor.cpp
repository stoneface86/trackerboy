
#include "InstrumentEditor.hpp"


using trackerboy::ChType;

static const uint8_t PANNING_TABLE[3] = {
    0x10, // Left
    0x01,  // right
    0x11, // Middle
};

InstrumentEditor::InstrumentEditor(InstrumentListModel &instModel, WaveListModel &waveModel, PianoInput &input, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mInstrumentModel(instModel),
    mWaveModel(waveModel),
    mIgnoreChanged(false),
    mLastChannel(ChType::ch4),
    mLayout(),
    mLayoutSelect(),
    mChannelCombo(),
    mInstrumentCombo(),
    mLayoutGroup(),
    mGroupSettings(tr("Settings")),
    mLayoutSettings(),
    mTimbreLabel(),
    mTimbreCombo(),
    mPanningCombo(),
    mDelaySpin(),
    mDurationSpin(),
    mGroupFrequency(tr("Frequency settings")),
    mLayoutFrequency(),
    mTuneSpin(),
    mVibratoSpeedSpin(),
    mVibratoExtentSpin(),
    mVibratoDelaySpin(),
    mGroupEnvelope(tr("Envelope")),
    mLayoutEnvelope(),
    mEnvelopeForm(),
    mGroupWave(tr("Waveform")),
    mLayoutWave(),
    mWaveCombo(),
    mWaveEditButton(tr("Edit")),
    mPiano(input)
{
    setWindowTitle(tr("Instrument Editor"));

    // layout
    mLayoutSelect.addWidget(&mChannelCombo);
    mLayoutSelect.addStretch(1);
    mLayoutSelect.addWidget(&mInstrumentCombo, 1);

    mLayoutSettings.addRow(&mTimbreLabel, &mTimbreCombo);
    mLayoutSettings.addRow(tr("Panning"), &mPanningCombo);
    mLayoutSettings.addRow(tr("Delay"), &mDelaySpin);
    mLayoutSettings.addRow(tr("Duration"), &mDurationSpin);
    mGroupSettings.setLayout(&mLayoutSettings);

    mLayoutFrequency.addRow(tr("Tune"), &mTuneSpin);
    mLayoutFrequency.addRow(tr("Vibrato speed"), &mVibratoSpeedSpin);
    mLayoutFrequency.addRow(tr("Vibrato extent"), &mVibratoExtentSpin);
    mLayoutFrequency.addRow(tr("Vibrato delay"), &mVibratoDelaySpin);
    mGroupFrequency.setLayout(&mLayoutFrequency);

    mLayoutEnvelope.setMargin(0);
    mLayoutEnvelope.addWidget(&mEnvelopeForm);
    mGroupEnvelope.setLayout(&mLayoutEnvelope);

    mLayoutWave.addWidget(&mWaveCombo);
    mLayoutWave.addWidget(&mWaveEditButton);
    mLayoutWave.addStretch();
    mGroupWave.setLayout(&mLayoutWave);

    mLayoutGroup.addWidget(&mGroupSettings);
    mLayoutGroup.addWidget(&mGroupFrequency);
    mLayoutGroup.addWidget(&mGroupEnvelope);
    mLayoutGroup.addWidget(&mGroupWave);

    mLayout.addLayout(&mLayoutSelect);
    mLayout.addLayout(&mLayoutGroup);
    mLayout.addWidget(&mPiano);
    setLayout(&mLayout);

    // prevent user from resizing
    mLayout.setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

    // settings
    updateTimbreCombo(ChType::ch1);
    mLastChannel = ChType::ch1;

    mChannelCombo.addItems({
        QStringLiteral("CH1"),
        QStringLiteral("CH2"),
        QStringLiteral("CH3"),
        QStringLiteral("CH4")
        });

    mInstrumentCombo.setModel(&instModel);

    mPanningCombo.addItems({
        tr("Left"),
        tr("Right"),
        tr("Middle")
        });

    mDelaySpin.setRange(0, 255);
    mDurationSpin.setRange(0, 255);

    mTuneSpin.setRange(-128, 127);
    mVibratoSpeedSpin.setRange(0, 15);
    mVibratoExtentSpin.setRange(0, 15);
    mVibratoDelaySpin.setRange(0, 255);

    mWaveCombo.setModel(&mWaveModel);
    mWaveEditButton.setEnabled(false);

    // only the envelope or waveform group box can be visible at once
    mGroupWave.setVisible(false);

    
    mWaveCombo.setModel(&waveModel);

    connect(&mChannelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InstrumentEditor::onChannelSelect);
    connect(&mInstrumentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), &mInstrumentModel, QOverload<int>::of(&InstrumentListModel::select));

    
    connect(&mPanningCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            if (!mIgnoreChanged) {
                mInstrumentModel.setPanning(PANNING_TABLE[index]);
            }
        });

    connect(&mDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            if (!mIgnoreChanged) {
                mInstrumentModel.setDelay(static_cast<uint8_t>(value));
            }
        });

    connect(&mDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setDuration(static_cast<uint8_t>(value));
        }
        });

    connect(&mTuneSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setTune(static_cast<int8_t>(value));
        }
        });

    auto vibratoChanged = [this](int value) {
        (void)value;
        if (!mIgnoreChanged) {
            mInstrumentModel.setVibrato(static_cast<uint8_t>(mVibratoExtentSpin.value()), static_cast<uint8_t>(mVibratoSpeedSpin.value()));
        }
    };
    connect(&mVibratoExtentSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, vibratoChanged);
    connect(&mVibratoSpeedSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, vibratoChanged);


    connect(&mVibratoDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setVibratoDelay(static_cast<uint8_t>(value));
        }
        });
  
    connect(&mTimbreCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setTimbre(static_cast<uint8_t>(value));
        }
        });

    connect(&mEnvelopeForm, &EnvelopeForm::envelopeChanged, this, [this](uint8_t envelope) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setEnvelope(envelope);
        }
        });

    connect(&mWaveCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        bool valid = index != -1;
        mWaveEditButton.setEnabled(valid);
        if (!mIgnoreChanged && mWaveCombo.isVisible()) {
            if (valid) {
                auto waveform = mWaveModel.waveform(index);
                uint8_t id = waveform->id();
                mInstrumentModel.setEnvelope(id);
            }
        }
        });

    connect(&mWaveEditButton, &QPushButton::clicked, this, [this]() {
        mWaveModel.select(mWaveCombo.currentIndex());
        emit waveEditorRequested();
        });

    connect(&mInstrumentModel, &InstrumentListModel::currentIndexChanged, this, &InstrumentEditor::currentInstrumentChanged);

    currentInstrumentChanged(mInstrumentModel.currentIndex());

}

InstrumentEditor::~InstrumentEditor() {
}

PianoWidget& InstrumentEditor::piano() {
    return mPiano;
}


void InstrumentEditor::onChannelSelect(int channel) {

    // the combobox's possible indices is the same as ChType
    ChType ch = static_cast<ChType>(channel);
    int timbre = mTimbreCombo.currentIndex();

    updateTimbreCombo(ch);

    if (timbre >= mTimbreCombo.count()) {
        timbre = mTimbreCombo.count() - 1;
    }
    mTimbreCombo.setCurrentIndex(timbre);

    // disable controls based on the selected channel

    // frequency effects do not work on CH4
    mGroupFrequency.setEnabled(ch != ChType::ch4);

    bool const isWaveChannel = ch == ChType::ch3;

    mGroupEnvelope.setVisible(!isWaveChannel);
    mGroupWave.setVisible(isWaveChannel);

    if (isWaveChannel) {
        mWaveCombo.setCurrentIndex(mWaveModel.idToModel(mEnvelopeForm.envelope()));
    } else if (mLastChannel == ChType::ch3) {
        int currentWave = mWaveCombo.currentIndex();
        if (currentWave != -1) {
            mEnvelopeForm.setEnvelope(mWaveModel.waveform(currentWave)->id());
        }
    }

    // update the current instrument's channel
    mInstrumentModel.setChannel(ch);

    mLastChannel = ch;
}

void InstrumentEditor::currentInstrumentChanged(int index) {
    if (index == -1) {
        close();
    } else {
        mInstrumentCombo.setCurrentIndex(index);
        
        auto inst = mInstrumentModel.instrument(index);
        auto &instData = inst->data();

        mChannelCombo.setCurrentIndex(instData.channel);

        mIgnoreChanged = true;

        int panningIndex = 0;
        for (int i = 0; i != 3; ++i) {
            if (PANNING_TABLE[i] == instData.panning) {
                break;
            }
            ++panningIndex;
        }
        mPanningCombo.setCurrentIndex(panningIndex);

        mDelaySpin.setValue(instData.delay);
        mDurationSpin.setValue(instData.duration);
        mTuneSpin.setValue(instData.tune);
        mVibratoExtentSpin.setValue(instData.vibrato & 0xF);
        mVibratoSpeedSpin.setValue(instData.vibrato >> 4);
        mVibratoDelaySpin.setValue(instData.vibratoDelay);

        mWaveCombo.setCurrentIndex(mWaveModel.idToModel(instData.envelope));
        mEnvelopeForm.setEnvelope(instData.envelope);
        

        mTimbreCombo.setCurrentIndex(instData.timbre);

        mIgnoreChanged = false;
    }
}


void InstrumentEditor::updateTimbreCombo(ChType ch) {
    switch (ch) {
        case ChType::ch1:
            if (mLastChannel == ChType::ch2) {
                break;
            }
            [[fallthrough]];
        case ChType::ch2:
            if (mLastChannel == ChType::ch1) {
                break;
            }
            mTimbreLabel.setText(tr("Duty"));
            mTimbreCombo.clear();
            mTimbreCombo.addItem(QStringLiteral("12.5%"));
            mTimbreCombo.addItem(QStringLiteral("25%"));
            mTimbreCombo.addItem(QStringLiteral("50%"));
            mTimbreCombo.addItem(QStringLiteral("75%"));
            break;
        case ChType::ch3:
            mTimbreLabel.setText(tr("Volume"));
            mTimbreCombo.clear();
            mTimbreCombo.addItem(tr("Mute"));
            mTimbreCombo.addItem(QStringLiteral("25%"));
            mTimbreCombo.addItem(QStringLiteral("50%"));
            mTimbreCombo.addItem(QStringLiteral("100%"));
            break;
        case ChType::ch4:
            mTimbreLabel.setText(tr("Step width"));
            mTimbreCombo.clear();
            mTimbreCombo.addItem(tr("15-bit"));
            mTimbreCombo.addItem(tr("7-bit"));
            break;
    }
}

