
#pragma once

#include <QMainWindow>
#include <QFileDialog>

#pragma warning(push, 0)
#include "designer/ui_MainWindow.h"
#pragma warning(pop)

#include "model/ModuleDocument.hpp"
#include "ConfigDialog.hpp"
#include "InstrumentEditor.hpp"
#include "WaveEditor.hpp"

class MainWindow : public QMainWindow, private Ui::MainWindow {

    Q_OBJECT

public:
    explicit MainWindow();

protected:

    void closeEvent(QCloseEvent *evt) override;

private slots:
    void waveformDoubleClicked(const QModelIndex &index);
    void updateWindowTitle();

    // actions
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();

private:

    // To be called before loading a new document. Prompts user to save if the
    // current document is modified. Returns false if the user does not want to continue
    bool maybeSave();

    void readSettings();

    void setFilename(QString filename);

    void writeSettings();

    QFileDialog *mModuleFileDialog;

    ModuleDocument *mDocument;

    InstrumentEditor *mInstrumentEditor;
    WaveEditor *mWaveEditor;
    ConfigDialog *mConfigDialog;

    // file name of the currently open file or "Untitled" for a new file
    QString mFilename;
    QString mDocumentName;

};
