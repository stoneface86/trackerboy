
#pragma once

#include "config/data/Palette.hpp"
#include "model/graph/WaveModel.hpp"
#include "model/TableModel.hpp"
#include "forms/editors/BaseEditor.hpp"
#include "widgets/GraphEdit.hpp"
#include "verdigris/wobjectdefs.h"

#include <QLineEdit>

#include <memory>


class WaveEditor : public BaseEditor {

    W_OBJECT(WaveEditor)

public:
    explicit WaveEditor(
        Module &mod,
        WaveListModel &model,
        PianoInput const& input,
        QWidget *parent = nullptr
    );

    void setColors(Palette const& pal);

protected:

    virtual void setCurrentItem(int index) override;

private:

    // downcasts BaseEditor::mModel to a WaveListModel
    WaveListModel& model() const noexcept;

    void setWaveFromPreset(int preset);
    
    WaveModel *mWaveModel;
    GraphEdit *mWaveEdit;
    QLineEdit *mWaveramEdit;
    bool mWaveEditedFromText;

    std::shared_ptr<trackerboy::Waveform> mWaveform;

};
