
#include "InstrumentEditor.hpp"

#include "trackerboy/ChType.hpp"
using trackerboy::ChType;

static const uint8_t PANNING_TABLE[3] = {
    0x10, // Left
    0x01,  // right
    0x11, // Middle
};

InstrumentEditor::InstrumentEditor(InstrumentListModel &instModel, WaveListModel &waveModel, QWidget &waveEditor, QWidget *parent) :
    mInstrumentModel(instModel),
    mWaveModel(waveModel),
    mWaveEditor(waveEditor),
    mIgnoreChanged(false),
    mLastChannel(ChType::ch1),
    QDialog(parent)
{
    setupUi(this);

    mGroupEnvelope = new QGroupBox("Envelope", this);

    QHBoxLayout *layout = new QHBoxLayout();
    mEnvelopeForm = new EnvelopeForm();
    layout->addWidget(mEnvelopeForm);
    layout->setMargin(0);
    mGroupEnvelope->setLayout(layout);

    mGroupLayout->replaceWidget(mGroupWave, mGroupEnvelope);
    mGroupWave->setVisible(false);

    mInstrumentCombo->setModel(&instModel);
    mWaveCombo->setModel(&waveModel);

    connect(mChannelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InstrumentEditor::onChannelSelect);
    connect(mInstrumentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), &mInstrumentModel, QOverload<int>::of(&InstrumentListModel::select));

    connect(mNameEdit, &QLineEdit::textEdited, &instModel, &InstrumentListModel::rename);

    connect(mPanningCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            if (!mIgnoreChanged) {
                mInstrumentModel.setPanning(PANNING_TABLE[index]);
            }
        });

    connect(mDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            if (!mIgnoreChanged) {
                mInstrumentModel.setDelay(static_cast<uint8_t>(value));
            }
        });

    connect(mDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setDuration(static_cast<uint8_t>(value));
        }
        });

    connect(mTuneSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setTune(static_cast<int8_t>(value));
        }
        });

    auto vibratoChanged = [this](int value) {
        (void)value;
        if (!mIgnoreChanged) {
            mInstrumentModel.setVibrato(static_cast<uint8_t>(mVibratoExtentSpin->value()), static_cast<uint8_t>(mVibratoSpeedSpin->value()));
        }
    };
    connect(mVibratoExtentSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, vibratoChanged);
    connect(mVibratoSpeedSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, vibratoChanged);


    connect(mVibratoDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setVibratoDelay(static_cast<uint8_t>(value));
        }
        });
  
    connect(mTimbreCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setTimbre(static_cast<uint8_t>(value));
        }
        });

    connect(mEnvelopeForm, &EnvelopeForm::envelopeChanged, this, [this](uint8_t envelope) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setEnvelope(envelope);
        }
        });

    connect(mWaveCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        bool valid = index != -1;
        mWaveEditButton->setEnabled(valid);
        if (!mIgnoreChanged && mWaveCombo->isVisible()) {
            if (valid) {
                auto waveform = mWaveModel.waveform(index);
                uint8_t id = waveform->id();
                mInstrumentModel.setEnvelope(id);
            }
        }
        });

    connect(mWaveEditButton, &QPushButton::clicked, this, [this]() {
        mWaveModel.select(mWaveCombo->currentIndex());
        mWaveEditor.show();
        });

    connect(&mInstrumentModel, &InstrumentListModel::currentIndexChanged, this, &InstrumentEditor::currentInstrumentChanged);
}


void InstrumentEditor::onChannelSelect(int channel) {

    // the combobox's possible indices is the same as ChType
    ChType ch = static_cast<ChType>(channel);

    // update timbre label and choices
    int timbre = mTimbreCombo->currentIndex();
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
            mTimbreLabel->setText("Duty");
            mTimbreCombo->clear();
            mTimbreCombo->addItem("12.5%");
            mTimbreCombo->addItem("25%");
            mTimbreCombo->addItem("50%");
            mTimbreCombo->addItem("75%");
            break;
        case ChType::ch3:
            mTimbreLabel->setText("Volume");
            mTimbreCombo->clear();
            mTimbreCombo->addItem("100%");
            mTimbreCombo->addItem("50%");
            mTimbreCombo->addItem("25%");
            mTimbreCombo->addItem("Mute");
            break;
        case ChType::ch4:
            mTimbreLabel->setText("Step width");
            mTimbreCombo->clear();
            mTimbreCombo->addItem("15-bit");
            mTimbreCombo->addItem("7-bit");
            break;
    }

    if (timbre >= mTimbreCombo->count()) {
        timbre = mTimbreCombo->count() - 1;
    }
    mTimbreCombo->setCurrentIndex(timbre);

    // disable controls based on the selected channel
    bool isFrequencyChannel = ch != ChType::ch4;
    mGroupFrequency->setEnabled(isFrequencyChannel);

    if (ch == ChType::ch3) {
        // replace the envelope group box with the waveform one
        mGroupLayout->replaceWidget(mGroupEnvelope, mGroupWave);
        mWaveCombo->setCurrentIndex(mWaveModel.idToModel(mEnvelopeForm->envelope()));
        mGroupEnvelope->setVisible(false);
        mGroupWave->setVisible(true);
    } else if (mLastChannel == ChType::ch3) {
        // replace the waveform group box with the envelope one
        mGroupLayout->replaceWidget(mGroupWave, mGroupEnvelope);
        
        int currentWave = mWaveCombo->currentIndex();
        if (currentWave != -1) {
            mEnvelopeForm->setEnvelope(mWaveModel.waveform(currentWave)->id());
        }
        mGroupEnvelope->setVisible(true);
        mGroupWave->setVisible(false);
    }

    // update the current instrument's channel
    mInstrumentModel.setChannel(ch);

    mLastChannel = ch;
}

void InstrumentEditor::currentInstrumentChanged(int index) {
    if (index != -1) {
        mInstrumentCombo->setCurrentIndex(index);
        mNameEdit->setText(mInstrumentModel.name());

        auto inst = mInstrumentModel.instrument(index);
        auto &instData = inst->data();

        mChannelCombo->setCurrentIndex(instData.channel);

        mIgnoreChanged = true;

        int panningIndex = 0;
        for (int i = 0; i != 3; ++i) {
            if (PANNING_TABLE[i] == instData.panning) {
                break;
            }
            ++panningIndex;
        }
        mPanningCombo->setCurrentIndex(panningIndex);

        mDelaySpin->setValue(instData.delay);
        mDurationSpin->setValue(instData.duration);
        mTuneSpin->setValue(instData.tune);
        mVibratoExtentSpin->setValue(instData.vibrato & 0xF);
        mVibratoSpeedSpin->setValue(instData.vibrato >> 4);
        mVibratoDelaySpin->setValue(instData.vibratoDelay);

        mWaveCombo->setCurrentIndex(mWaveModel.idToModel(instData.envelope));
        mEnvelopeForm->setEnvelope(instData.envelope);
        

        mTimbreCombo->setCurrentIndex(instData.timbre);

        mIgnoreChanged = false;
    }
}




