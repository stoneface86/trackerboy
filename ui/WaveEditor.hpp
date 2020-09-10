
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
    explicit WaveEditor(WaveListModel &model, QWidget *parent = nullptr);


private slots:
    void onSampleChanged(QPoint sample);
    void onWaveramEdited(const QString &text);
    void selectionChanged(int index);
    void nameEdited(const QString &text);

private:

    enum class Preset {
        square,
        triangle,
        sawtooth,
        sine
    };

    void updateWaveramText();

    void setFromPreset(Preset preset);

    //ModuleDocument *mDocument;
    WaveListModel &mModel;

    bool mIgnoreNextUpdate;


    


};
