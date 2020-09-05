
#pragma once

#include <array>

#include <QWidget>

#include "model/ModuleDocument.hpp"
#include "model/WaveListModel.hpp"

#pragma warning(push, 0)
#include "designer/ui_WaveEditor.h"
#pragma warning(pop)

class WaveEditor : public QWidget, private Ui::WaveEditor {

    Q_OBJECT

public:
    explicit WaveEditor(ModuleDocument *doc, QWidget *parent = nullptr);

public slots:
    void selectWaveform(const QModelIndex &index);


private slots:
    void onSampleChanged(QPoint sample);
    void onWaveramEdited(const QString &text);
    void selectionChanged(int index);
    void nameEdited(const QString &text);
    
    // function buttons
    void onRotateLeft();
    void onRotateRight();
    void onInvert();
    void onClear();

private:

    void pack();

    enum class Preset {
        square,
        triangle,
        sawtooth,
        sine
    };

    void updateWaveramText();

    void setFromPreset(Preset preset);

    //trackerboy::Module &mMod;
    ModuleDocument *mDocument;
    trackerboy::Waveform *mCurrentWaveform;

    // unpacked version of the waveform data for convenience
    std::array<uint8_t, 32> mWavedata;


    


};
