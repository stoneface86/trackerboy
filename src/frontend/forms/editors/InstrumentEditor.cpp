
#include "forms/editors/InstrumentEditor.hpp"

#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>

#define TU InstrumentEditorTU
namespace TU {

static auto SET_VOLUME_CHECK_STR = QT_TR_NOOP("Set volume on trigger");
static auto SET_WAVE_CHECK_STR = QT_TR_NOOP("Set waveform on trigger");
static auto DUTY_CYCLE_STR = QT_TR_NOOP("Duty cycle");

}

InstrumentEditor::InstrumentEditor(
    Module &mod,
    InstrumentListModel &model,
    WaveListModel &waveListModel,
    PianoInput const& input,
    QWidget *parent
) :
    BaseEditor(model, input, parent),
    mModule(mod),
    mWaveListModel(waveListModel),
    mChannelCombo(nullptr),
    mSetEnvelopeCheck(nullptr),
    mEnvelopeGroup(nullptr),
    mEnvelopeForm(nullptr),
    mWaveformGroup(nullptr),
    mWaveformCombo(nullptr),
    mWaveformEditButton(nullptr),
    mSequenceTabs(nullptr),
    mSequenceEditors(),
    mInstrument(nullptr),
    mCanEdit(true)
{
    setWindowTitle(tr("Instrument editor"));
    auto channelGroup = new QGroupBox(tr("Channel"));
    {
        auto layout = new QVBoxLayout;
        mChannelCombo = new QComboBox;
        mSetEnvelopeCheck = new QCheckBox(tr(TU::SET_VOLUME_CHECK_STR));
        layout->addWidget(mChannelCombo);
        layout->addWidget(mSetEnvelopeCheck);
        layout->addStretch();
        channelGroup->setLayout(layout);
    }

    mEnvelopeGroup = new QGroupBox(tr("Volume envelope"));
    {
        auto layout = new QVBoxLayout;
        mEnvelopeForm = new EnvelopeForm;
        layout->addWidget(mEnvelopeForm);
        layout->addStretch();
        layout->setContentsMargins(0, 0, 0, 0);
        mEnvelopeGroup->setLayout(layout);
    }

    mWaveformGroup = new QGroupBox(tr("Waveform"));
    {
        auto layout = new QHBoxLayout;
        mWaveformCombo = new QComboBox;
        mWaveformCombo->setModel(&mWaveListModel);
        mWaveformEditButton = new QPushButton(tr("Edit"));
        mWaveformEditButton->setAutoDefault(false);
        mWaveformEditButton->setDefault(false);
        layout->addWidget(mWaveformCombo, 1);
        layout->addWidget(mWaveformEditButton);
        layout->setAlignment(Qt::AlignTop);
        mWaveformGroup->setLayout(layout);

        
    }

    mSequenceTabs = new QTabWidget;

    auto layout = new QVBoxLayout;
    auto settingsLayout = new QHBoxLayout;
    settingsLayout->addWidget(channelGroup);
    settingsLayout->addWidget(mEnvelopeGroup, 1);
    settingsLayout->addWidget(mWaveformGroup, 1);
    layout->addLayout(settingsLayout);
    layout->addWidget(mSequenceTabs, 1);
    layout->setContentsMargins(0, 0, 0, 0);
    editorWidget()->setLayout(layout);
    
    mWaveformGroup->hide();
    mWaveformGroup->setEnabled(false);
    mEnvelopeGroup->setEnabled(false);
    
    mChannelCombo->addItems({
        tr("CH1 - Duty"),
        tr("CH2 - Duty"),
        tr("CH3 - Wave"),
        tr("CH4 - Noise")
    });

    for (size_t i = 0; i < trackerboy::Instrument::SEQUENCE_COUNT; ++i) {
        mSequenceEditors[i] = new SequenceEditor(mod, i);
    }
    

    mSequenceTabs->addTab(mSequenceEditors[0], tr("Arpeggio"));
    mSequenceTabs->addTab(mSequenceEditors[1], tr("Panning"));
    mSequenceTabs->addTab(mSequenceEditors[2], tr("Pitch"));
    mSequenceTabs->addTab(mSequenceEditors[3], tr(TU::DUTY_CYCLE_STR));
    

    connect(mSetEnvelopeCheck, &QCheckBox::toggled, this,
        [this](bool checked) {
            mEnvelopeGroup->setEnabled(checked);
            mWaveformGroup->setEnabled(checked);
            setEnvelopeEnable(checked);
        });

    connect(mChannelCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
        [this](int index) {
            if (index == -1) {
                return;
            }
            setChannel(index);
            
            auto const isWave = index == 2;
            mWaveformGroup->setVisible(isWave);
            mEnvelopeGroup->setVisible(!isWave);

            switch (index) {
                case 0:
                case 1:
                    mSequenceTabs->setTabText(3, tr(TU::DUTY_CYCLE_STR));
                    break;
                case 2:
                    mSequenceTabs->setTabText(3, tr("Volume"));
                    break;
                case 3:
                    mSequenceTabs->setTabText(3, tr("Noise"));
                    break;
            }

            if (isWave) {
                setEnvelopeToWaveform(mWaveformCombo->currentIndex());
                mSetEnvelopeCheck->setText(tr(TU::SET_WAVE_CHECK_STR));
            } else {
                setEnvelope(mEnvelopeForm->envelope());
                mSetEnvelopeCheck->setText(tr(TU::SET_VOLUME_CHECK_STR));
            }

        });

    
    connect(mWaveformCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &InstrumentEditor::setEnvelopeToWaveform);

    connect(mWaveformEditButton, &QPushButton::clicked, this,
        [this]() {
            emit openWaveEditor(mWaveformCombo->currentIndex());
        });

    connect(mEnvelopeForm, &EnvelopeForm::envelopeChanged, this, &InstrumentEditor::setEnvelope);

}


void InstrumentEditor::setChannel(int channel) {
    auto chtype = static_cast<trackerboy::ChType>(channel);
    if (mCanEdit && chtype != mInstrument->channel()) {
        auto ctx = mModule.permanentEdit();
        mInstrument->setChannel(chtype);
        model().updateChannelIcon(currentItem());
    }

}

void InstrumentEditor::setColors(const Palette &pal) {
    for (auto editor : mSequenceEditors) {
        editor->graphEdit()->setColors(pal);
    }
}

void InstrumentEditor::setCurrentItem(int index) {
    mInstrument = model().getShared(index);
    for (auto editor : mSequenceEditors) {
        editor->setInstrument(mInstrument.get());
    }
    
    if (mInstrument) {

        mCanEdit = false;
        auto channel = mInstrument->channel();
        mChannelCombo->setCurrentIndex(static_cast<int>(channel));

        bool const hasEnvelope = mInstrument->hasEnvelope();
        mSetEnvelopeCheck->setChecked(hasEnvelope);
        mEnvelopeGroup->setEnabled(hasEnvelope);
        mWaveformGroup->setEnabled(hasEnvelope);


        auto envelope = mInstrument->envelope();
        if (channel == trackerboy::ChType::ch3) {
            QSignalBlocker blocker(mWaveformCombo);
            mWaveformCombo->setCurrentIndex(mWaveListModel.lookupId(envelope));
        } else {
            QSignalBlocker blocker(mEnvelopeForm);
            mEnvelopeForm->setEnvelope(envelope);
        }
        mCanEdit = true;
        
    }

}

void InstrumentEditor::setEnvelopeToWaveform(int index) {
    auto const hasIndex = index != -1;
    mWaveformEditButton->setEnabled(hasIndex);
    if (hasIndex && mWaveformGroup->isVisible()) {
        setEnvelope(mWaveListModel.id(index));
    }
}

void InstrumentEditor::setEnvelope(uint8_t envelope) {
    if (mCanEdit && mInstrument->envelope() != envelope) {
        auto ctx = mModule.permanentEdit();
        mInstrument->setEnvelope(envelope);
    }
}

void InstrumentEditor::setEnvelopeEnable(bool enabled) {
    if (mCanEdit && mInstrument->hasEnvelope() != enabled) {
        auto ctx = mModule.permanentEdit();
        mInstrument->setEnvelopeEnable(enabled);
    }
}

InstrumentListModel& InstrumentEditor::model() const noexcept {
    return static_cast<InstrumentListModel&>(mModel);
}

#undef TU
