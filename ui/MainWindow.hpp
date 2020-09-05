
#pragma once

#include <QMainWindow>

#pragma warning(push, 0)
#include "designer/ui_MainWindow.h"
#pragma warning(pop)

#include "model/ModuleDocument.hpp"
#include "InstrumentEditor.hpp"
#include "WaveEditor.hpp"

class MainWindow : public QMainWindow, private Ui::MainWindow {

    Q_OBJECT

public:
    explicit MainWindow();

private slots:
    void waveformDoubleClicked(const QModelIndex &index);

private:

    ModuleDocument *mDocument;

    InstrumentEditor *mInstrumentEditor;
    WaveEditor *mWaveEditor;

};
