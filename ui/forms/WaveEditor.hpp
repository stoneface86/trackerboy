
#pragma once

#include "model/ModuleDocument.hpp"
#include "model/WaveListModel.hpp"
#include "widgets/PianoWidget.hpp"

#include <QDialog>

#include <array>

namespace Ui {
class WaveEditor;
}

class WaveEditor : public QDialog {

    Q_OBJECT

public:
    explicit WaveEditor(WaveListModel &model, QWidget *parent = nullptr);
    ~WaveEditor();

    PianoWidget* piano();

private slots:
    void onSampleChanged(QPoint sample);
    void onWaveramEdited(const QString &text);
    void selectionChanged(int index);

private:

    enum class Preset {
        square,
        triangle,
        sawtooth,
        sine
    };

    void updateWaveramText();

    void setFromPreset(Preset preset);

    Ui::WaveEditor *mUi;

    WaveListModel &mModel;

    bool mIgnoreNextUpdate;


    


};
