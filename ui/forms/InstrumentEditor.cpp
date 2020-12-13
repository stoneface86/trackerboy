
#include "InstrumentEditor.hpp"

#pragma warning(push, 0)
#include "ui_InstrumentEditor.h"
#pragma warning(pop)

#include "trackerboy/ChType.hpp"
using trackerboy::ChType;

static const uint8_t PANNING_TABLE[3] = {
    0x10, // Left
    0x01,  // right
    0x11, // Middle
};

InstrumentEditor::InstrumentEditor(InstrumentListModel &instModel, WaveListModel &waveModel, QWidget &waveEditor, QWidget *parent) :
    mUi(new Ui::InstrumentEditor()),
    mInstrumentModel(instModel),
    mWaveModel(waveModel),
    mWaveEditor(waveEditor),
    mIgnoreChanged(false),
    mLastChannel(ChType::ch1),
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    mUi->setupUi(this);

    // only the envelope or waveform group box can be visible at once
    mUi->mGroupWave->setVisible(false);

    mUi->mInstrumentCombo->setModel(&instModel);
    mUi->mWaveCombo->setModel(&waveModel);

    connect(mUi->mChannelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InstrumentEditor::onChannelSelect);
    connect(mUi->mInstrumentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), &mInstrumentModel, QOverload<int>::of(&InstrumentListModel::select));

    connect(mUi->mNameEdit, &QLineEdit::textEdited, &instModel, qOverload<const QString&>(&InstrumentListModel::rename));

    connect(mUi->mPanningCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            if (!mIgnoreChanged) {
                mInstrumentModel.setPanning(PANNING_TABLE[index]);
            }
        });

    connect(mUi->mDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
            if (!mIgnoreChanged) {
                mInstrumentModel.setDelay(static_cast<uint8_t>(value));
            }
        });

    connect(mUi->mDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setDuration(static_cast<uint8_t>(value));
        }
        });

    connect(mUi->mTuneSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setTune(static_cast<int8_t>(value));
        }
        });

    auto vibratoChanged = [this](int value) {
        (void)value;
        if (!mIgnoreChanged) {
            mInstrumentModel.setVibrato(static_cast<uint8_t>(mUi->mVibratoExtentSpin->value()), static_cast<uint8_t>(mUi->mVibratoSpeedSpin->value()));
        }
    };
    connect(mUi->mVibratoExtentSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, vibratoChanged);
    connect(mUi->mVibratoSpeedSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, vibratoChanged);


    connect(mUi->mVibratoDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setVibratoDelay(static_cast<uint8_t>(value));
        }
        });
  
    connect(mUi->mTimbreCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int value) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setTimbre(static_cast<uint8_t>(value));
        }
        });

    connect(mUi->mEnvelopeForm, &EnvelopeForm::envelopeChanged, this, [this](uint8_t envelope) {
        if (!mIgnoreChanged) {
            mInstrumentModel.setEnvelope(envelope);
        }
        });

    connect(mUi->mWaveCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        bool valid = index != -1;
        mUi->mWaveEditButton->setEnabled(valid);
        if (!mIgnoreChanged && mUi->mWaveCombo->isVisible()) {
            if (valid) {
                auto waveform = mWaveModel.waveform(index);
                uint8_t id = waveform->id();
                mInstrumentModel.setEnvelope(id);
            }
        }
        });

    connect(mUi->mWaveEditButton, &QPushButton::clicked, this, [this]() {
        mWaveModel.select(mUi->mWaveCombo->currentIndex());
        mWaveEditor.show();
        });

    connect(&mInstrumentModel, &InstrumentListModel::currentIndexChanged, this, &InstrumentEditor::currentInstrumentChanged);

    // prevent the user from resizing the window
    setFixedSize(size());
}

InstrumentEditor::~InstrumentEditor() {
    delete mUi;
}

PianoWidget* InstrumentEditor::piano() {
    return mUi->mPiano;
}


void InstrumentEditor::onChannelSelect(int channel) {

    // the combobox's possible indices is the same as ChType
    ChType ch = static_cast<ChType>(channel);

    // update timbre label and choices
    int timbre = mUi->mTimbreCombo->currentIndex();
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
            mUi->mTimbreLabel->setText("Duty");
            mUi->mTimbreCombo->clear();
            mUi->mTimbreCombo->addItem("12.5%");
            mUi->mTimbreCombo->addItem("25%");
            mUi->mTimbreCombo->addItem("50%");
            mUi->mTimbreCombo->addItem("75%");
            break;
        case ChType::ch3:
            mUi->mTimbreLabel->setText("Volume");
            mUi->mTimbreCombo->clear();
            mUi->mTimbreCombo->addItem("Mute");
            mUi->mTimbreCombo->addItem("25%");
            mUi->mTimbreCombo->addItem("50%");
            mUi->mTimbreCombo->addItem("100%");
            break;
        case ChType::ch4:
            mUi->mTimbreLabel->setText("Step width");
            mUi->mTimbreCombo->clear();
            mUi->mTimbreCombo->addItem("15-bit");
            mUi->mTimbreCombo->addItem("7-bit");
            break;
    }

    if (timbre >= mUi->mTimbreCombo->count()) {
        timbre = mUi->mTimbreCombo->count() - 1;
    }
    mUi->mTimbreCombo->setCurrentIndex(timbre);

    // disable controls based on the selected channel

    // frequency effects do not work on CH4
    mUi->mGroupFrequency->setEnabled(ch != ChType::ch4);

    bool const isWaveChannel = ch == ChType::ch3;

    mUi->mGroupEnvelope->setVisible(!isWaveChannel);
    mUi->mGroupWave->setVisible(isWaveChannel);

    if (isWaveChannel) {
        mUi->mWaveCombo->setCurrentIndex(mWaveModel.idToModel(mUi->mEnvelopeForm->envelope()));
    } else if (mLastChannel == ChType::ch3) {
        int currentWave = mUi->mWaveCombo->currentIndex();
        if (currentWave != -1) {
            mUi->mEnvelopeForm->setEnvelope(mWaveModel.waveform(currentWave)->id());
        }
    }

    // update the current instrument's channel
    mInstrumentModel.setChannel(ch);

    mLastChannel = ch;
}

void InstrumentEditor::currentInstrumentChanged(int index) {
    if (index != -1) {
        mUi->mInstrumentCombo->setCurrentIndex(index);
        mUi->mNameEdit->setText(mInstrumentModel.name());

        auto inst = mInstrumentModel.instrument(index);
        auto &instData = inst->data();

        mUi->mChannelCombo->setCurrentIndex(instData.channel);

        mIgnoreChanged = true;

        int panningIndex = 0;
        for (int i = 0; i != 3; ++i) {
            if (PANNING_TABLE[i] == instData.panning) {
                break;
            }
            ++panningIndex;
        }
        mUi->mPanningCombo->setCurrentIndex(panningIndex);

        mUi->mDelaySpin->setValue(instData.delay);
        mUi->mDurationSpin->setValue(instData.duration);
        mUi->mTuneSpin->setValue(instData.tune);
        mUi->mVibratoExtentSpin->setValue(instData.vibrato & 0xF);
        mUi->mVibratoSpeedSpin->setValue(instData.vibrato >> 4);
        mUi->mVibratoDelaySpin->setValue(instData.vibratoDelay);

        mUi->mWaveCombo->setCurrentIndex(mWaveModel.idToModel(instData.envelope));
        mUi->mEnvelopeForm->setEnvelope(instData.envelope);
        

        mUi->mTimbreCombo->setCurrentIndex(instData.timbre);

        mIgnoreChanged = false;
    }
}




