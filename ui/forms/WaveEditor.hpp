
#pragma once

#include <array>
#include <memory>

#include <QDialog>

#include "model/ModuleDocument.hpp"
#include "model/WaveListModel.hpp"
#include "PianoWidget.hpp"


namespace Ui {
class WaveEditor;
}

class WaveEditor : public QDialog {

    Q_OBJECT

public:
    explicit WaveEditor(WaveListModel &model, QWidget *parent = nullptr);

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

    std::unique_ptr<Ui::WaveEditor> mUi;

    WaveListModel &mModel;

    bool mIgnoreNextUpdate;


    


};
