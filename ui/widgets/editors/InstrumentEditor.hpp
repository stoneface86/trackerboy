
#pragma once

#include "core/model/graph/SequenceModel.hpp"
#include "widgets/editors/BaseEditor.hpp"
#include "widgets/EnvelopeForm.hpp"
#include "widgets/SequenceEditor.hpp"

#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QTabBar>
#include <QVBoxLayout>

class InstrumentEditor : public BaseEditor {

    Q_OBJECT

public:
    explicit InstrumentEditor(PianoInput const& input, QWidget *parent = nullptr);

signals:
    void openWaveEditor(int index);

protected:

    virtual void documentChanged(ModuleDocument *doc) override;

    virtual void setCurrentItem(int index) override;

    virtual BaseTableModel* getModel(ModuleDocument &doc) override;

private slots:
    void setChannel(int channel);

    void setEnvelopeToWaveform(int index);

    void setEnvelope(uint8_t envelope);

    void setEnvelopeEnable(bool enabled);

private:
    void setSequenceInModel(int index);

    QVBoxLayout mLayout;
        // channel settings
        QHBoxLayout mSettingsLayout;
            QGroupBox mChannelGroup;
                QVBoxLayout mChannelLayout;
                    QComboBox mChannelCombo;
                    QCheckBox mSetEnvelopeCheck;
            QGroupBox mEnvelopeGroup;
                QVBoxLayout mEnvelopeLayout;
                    EnvelopeForm mEnvelopeForm;
            QGroupBox mWaveformGroup;
                QHBoxLayout mWaveformLayout;
                    QComboBox mWaveformCombo;
                    QPushButton mWaveformEditButton;
        
        // sequence editors
        QTabWidget mSequenceTabs;
            std::array<SequenceEditor, 4> mSequenceEditors;

    trackerboy::Instrument *mInstrument;
    bool mCanModifyInstrument;

};
