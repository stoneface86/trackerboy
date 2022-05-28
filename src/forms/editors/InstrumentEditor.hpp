
#pragma once

#include "model/TableModel.hpp"
#include "config/data/Palette.hpp"
#include "forms/editors/BaseEditor.hpp"
#include "widgets/EnvelopeForm.hpp"
#include "widgets/SequenceEditor.hpp"
#include "verdigris/wobjectdefs.h"

#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>

class InstrumentEditor : public BaseEditor {

    W_OBJECT(InstrumentEditor)

public:
    InstrumentEditor(
        Module &mod,
        InstrumentListModel &model,
        WaveListModel &waveListModel,
        PianoInput const& input,
        QWidget *parent = nullptr
    );

    void setColors(Palette const& pal);

//signals:
    void openWaveEditor(int index) W_SIGNAL(openWaveEditor, index)

protected:
    virtual void setCurrentItem(int index) override;

private:
    Q_DISABLE_COPY(InstrumentEditor)

    void setChannel(int channel);

    void setEnvelopeToWaveform(int index);

    void setEnvelope(uint8_t envelope);

    void setEnvelopeEnable(bool enabled);

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
