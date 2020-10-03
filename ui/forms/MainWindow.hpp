
#pragma once

#include "forms/ConfigDialog.hpp"
#include "forms/InstrumentEditor.hpp"
#include "forms/WaveEditor.hpp"
#include "model/ModuleDocument.hpp"
#include "model/InstrumentListModel.hpp"
#include "model/SongListModel.hpp"
#include "model/WaveListModel.hpp"
#include "Renderer.hpp"

#include <QMainWindow>
#include <QFileDialog>
#include <QComboBox>
#include <QToolBar>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow();
    ~MainWindow();

protected:

    void closeEvent(QCloseEvent *evt) override;

private slots:
    void updateWindowTitle();

    // actions
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void moduleRemoveSong();
    void windowResetLayout();

private:

    // To be called before loading a new document. Prompts user to save if the
    // current document is modified. Returns false if the user does not want to continue
    bool maybeSave();

    void readSettings();

    void setFilename(QString filename);

    // enable or disable all models
    void setModelsEnabled(bool enabled);
    
    void setupConnections();
    
    void setupUi();

    void writeSettings();

    Ui::MainWindow *mUi;

    QFileDialog *mModuleFileDialog;

    Config *mConfig;
    ModuleDocument *mDocument;
    InstrumentListModel *mInstrumentModel;
    SongListModel *mSongModel;
    WaveListModel *mWaveModel;
    
    WaveEditor *mWaveEditor;
    InstrumentEditor *mInstrumentEditor;
    ConfigDialog *mConfigDialog;

    Renderer *mRenderer;

    // file name of the currently open file or "Untitled" for a new file
    QString mFilename;
    QString mDocumentName;

    // qwidgets not set in the ui file

    QToolBar *mSongToolbar = nullptr;
    QComboBox *mSongCombo = nullptr;

};
