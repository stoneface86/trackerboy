
#pragma once

#include "core/Palette.hpp"
#include "core/model/graph/WaveModel.hpp"
#include "core/model/TableModel.hpp"
#include "forms/editors/BaseEditor.hpp"
#include "widgets/GraphEdit.hpp"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>


class WaveEditor : public BaseEditor {

    Q_OBJECT

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
