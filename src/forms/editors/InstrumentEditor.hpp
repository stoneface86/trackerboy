
#pragma once

#include "core/model/graph/SequenceModel.hpp"
#include "core/model/TableModel.hpp"
#include "forms/editors/BaseEditor.hpp"
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
    InstrumentEditor(
        Module &mod,
        InstrumentListModel &model,
        WaveListModel &waveListModel,
        PianoInput const& input,
        QWidget *parent = nullptr
    );

signals:
    void openWaveEditor(int index);

protected:
    virtual void setCurrentItem(int index) override;

private slots:
    void setChannel(int channel);

    void setEnvelopeToWaveform(int index);

    void setEnvelope(uint8_t envelope);

    void setEnvelopeEnable(bool enabled);

private:
    Q_DISABLE_COPY(InstrumentEditor)

    InstrumentListModel& model() const noexcept;

    void setSequenceInModel(int index);

    Module &mModule;
    WaveListModel &mWaveListModel;

    QComboBox *mChannelCombo;
    QCheckBox *mSetEnvelopeCheck;
    QGroupBox *mEnvelopeGroup;
    EnvelopeForm *mEnvelopeForm;
    
    QGroupBox *mWaveformGroup;
    QComboBox *mWaveformCombo;
    QPushButton *mWaveformEditButton;

    QTabWidget *mSequenceTabs;
    std::array<SequenceEditor*, 4> mSequenceEditors;

    std::shared_ptr<trackerboy::Instrument> mInstrument;
    bool mCanEdit;

};
