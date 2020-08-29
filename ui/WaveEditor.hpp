
#pragma once

#include <array>

#include <QWidget>

#pragma warning(push, 0)
#include "designer/ui_waveeditor.h"
#pragma warning(pop)

class WaveEditor : public QWidget, private Ui::WaveEditor {

    Q_OBJECT

public:
    explicit WaveEditor();

private slots:
    void onSampleChanged(QPoint sample);
    void onWaveramEdited(const QString &text);
    
    // function buttons
    void onRotateLeft();
    void onRotateRight();
    void onInvert();
    void onClear();

private:

    enum class Preset {
        square,
        triangle,
        sawtooth,
        sine
    };

    void updateWaveramText();

    void setFromPreset(Preset preset);

    // unpacked version of the waveform data for convenience
    std::array<uint8_t, 32> mWavedata;


    


};
