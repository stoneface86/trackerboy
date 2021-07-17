
#include "widgets/editors/InstrumentEditor.hpp"

#include <algorithm>

static auto SET_VOLUME_CHECK_STR = QT_TR_NOOP("Set volume on trigger");
static auto SET_WAVE_CHECK_STR = QT_TR_NOOP("Set waveform on trigger");
static auto DUTY_CYCLE_STR = QT_TR_NOOP("Duty cycle");

InstrumentEditor::InstrumentEditor(PianoInput const& input, QWidget *parent) :
    BaseEditor(input, tr("instrument"), parent),
    mLayout(),
    mSettingsLayout(),
    mChannelGroup(tr("Channel")),
    mChannelLayout(),
    mChannelCombo(),
    mSetEnvelopeCheck(tr(SET_VOLUME_CHECK_STR)),
    mEnvelopeGroup(tr("Volume envelope")),
    mEnvelopeLayout(),
    mEnvelopeForm(),
    mWaveformGroup(tr("Waveform")),
    mWaveformLayout(),
    mWaveformCombo(),
    mWaveformEditButton(tr("Edit")),
    mSequenceTabs(),
    mSequenceEditors{
        SequenceEditor(trackerboy::Instrument::SEQUENCE_ARP),
        SequenceEditor(trackerboy::Instrument::SEQUENCE_PANNING),
        SequenceEditor(trackerboy::Instrument::SEQUENCE_PITCH),
        SequenceEditor(trackerboy::Instrument::SEQUENCE_TIMBRE)
    },
    mInstrument(nullptr),
    mCanModifyInstrument(false)
{

    mChannelLayout.addWidget(&mChannelCombo);
    mChannelLayout.addWidget(&mSetEnvelopeCheck);
    mChannelLayout.addStretch();
    mChannelGroup.setLayout(&mChannelLayout);

    mEnvelopeLayout.addWidget(&mEnvelopeForm);
    mEnvelopeLayout.addStretch();
    mEnvelopeLayout.setMargin(0);
    mEnvelopeGroup.setLayout(&mEnvelopeLayout);

    mWaveformLayout.addWidget(&mWaveformCombo, 1);
    mWaveformLayout.addWidget(&mWaveformEditButton);
    mWaveformLayout.setAlignment(Qt::AlignTop);
    mWaveformGroup.setLayout(&mWaveformLayout);

    mWaveformGroup.setEnabled(false);
    mEnvelopeGroup.setEnabled(false);

    mSettingsLayout.addWidget(&mChannelGroup);
    mSettingsLayout.addWidget(&mEnvelopeGroup, 1);
    mSettingsLayout.addWidget(&mWaveformGroup, 1);
    mWaveformGroup.hide();

    mLayout.addLayout(&mSettingsLayout);
    mLayout.addWidget(&mSequenceTabs, 1);
    mLayout.setMargin(0);

    auto &_editorWidget = editorWidget();
    _editorWidget.setLayout(&mLayout);
    
    mChannelCombo.addItems({
        QStringLiteral("CH1 - Duty"),
        QStringLiteral("CH2 - Duty"),
        QStringLiteral("CH3 - Wave"),
        QStringLiteral("CH4 - Noise")
    });

    mSequenceTabs.addTab(&mSequenceEditors[0], tr("Arpeggio"));
    mSequenceTabs.addTab(&mSequenceEditors[1], tr("Panning"));
    mSequenceTabs.addTab(&mSequenceEditors[2], tr("Pitch"));
    mSequenceTabs.addTab(&mSequenceEditors[3], tr(DUTY_CYCLE_STR));

    //mGraph.setViewMode(GraphEdit::ArpeggioView);

    

    connect(&mSetEnvelopeCheck, &QCheckBox::toggled, this,
        [this](bool checked) {
            mEnvelopeGroup.setEnabled(checked);
            mWaveformGroup.setEnabled(checked);
            setEnvelopeEnable(checked);
        });

    connect(&mChannelCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
        [this](int index) {
            if (index == -1) {
                return;
            }
            setChannel(index);
            
            auto const isWave = index == 2;
            mWaveformGroup.setVisible(isWave);
            mEnvelopeGroup.setVisible(!isWave);

            switch (index) {
                case 0:
                case 1:
                    mSequenceTabs.setTabText(3, tr(DUTY_CYCLE_STR));
                    break;
                case 2:
                    mSequenceTabs.setTabText(3, tr("Volume"));
                    break;
                case 3:
                    mSequenceTabs.setTabText(3, tr("Noise"));
                    break;
            }

            if (isWave) {
                setEnvelopeToWaveform(mWaveformCombo.currentIndex());
                mSetEnvelopeCheck.setText(tr(SET_WAVE_CHECK_STR));
            } else {
                setEnvelope(mEnvelopeForm.envelope());
                mSetEnvelopeCheck.setText(tr(SET_VOLUME_CHECK_STR));
            }

        });

    connect(&mWaveformEditButton, &QPushButton::clicked, this,
        [this]() {
            emit openWaveEditor(mWaveformCombo.currentIndex());
        });
    connect(&mWaveformCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &InstrumentEditor::setEnvelopeToWaveform);

    connect(&mEnvelopeForm, &EnvelopeForm::envelopeChanged, this, &InstrumentEditor::setEnvelope);

}

void InstrumentEditor::documentChanged(Document *doc) {
    if (doc == nullptr) {
        mWaveformCombo.clear();
    } else {
        mWaveformCombo.setModel(&doc->waveModel());
    }
}

void InstrumentEditor::setChannel(int channel) {
    if (mCanModifyInstrument) {
        auto chtype = static_cast<trackerboy::ChType>(channel);
        auto doc = document();
        if (chtype != mInstrument->channel()) {
            auto ctx = doc->permanentEdit();
            mInstrument->setChannel(chtype);
        }
        doc->instrumentModel().updateChannelIcon();
    }
}

void InstrumentEditor::setCurrentItem(int index) {
    auto doc = document();
    if (index == -1) {
        for (auto &editor : mSequenceEditors) {
            editor.removeInstrument();
        }
        mInstrument = nullptr;
        mCanModifyInstrument = false;
    } else {
        mInstrument = doc->mod().instrumentTable().get(doc->instrumentModel().id(index));
        for (auto &editor : mSequenceEditors) {
            editor.setInstrument(doc, mInstrument);
        }


        mCanModifyInstrument = false;
        auto channel = mInstrument->channel();
        mChannelCombo.setCurrentIndex(static_cast<int>(channel));
        mSetEnvelopeCheck.setChecked(mInstrument->hasEnvelope());
        auto envelope = mInstrument->envelope();
        if (channel == trackerboy::ChType::ch3) {
            mWaveformCombo.setCurrentIndex(doc->waveModel().lookupId(envelope));
        } else {
            mEnvelopeForm.setEnvelope(envelope);
        }
        
        mCanModifyInstrument = true;
    }

}

BaseTableModel* InstrumentEditor::getModel(Document &doc) {
    return &doc.instrumentModel();
}

void InstrumentEditor::setEnvelopeToWaveform(int index) {
    auto const hasIndex = index != -1;
    mWaveformEditButton.setEnabled(hasIndex);
    if (mCanModifyInstrument && hasIndex && mWaveformGroup.isVisible()) {
        setEnvelope(document()->waveModel().id(index));
    }
}

void InstrumentEditor::setEnvelope(uint8_t envelope) {
    if (mCanModifyInstrument && mInstrument->envelope() != envelope) {
        auto ctx = document()->permanentEdit();
        mInstrument->setEnvelope(envelope);
    }
}

void InstrumentEditor::setEnvelopeEnable(bool enabled) {
    if (mCanModifyInstrument && mInstrument->hasEnvelope() != enabled) {
        auto ctx = document()->permanentEdit();
        mInstrument->setEnvelopeEnable(enabled);
    }
}


