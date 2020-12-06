
#pragma once

#include "forms/ConfigDialog.hpp"
#include "forms/InstrumentEditor.hpp"
#include "forms/WaveEditor.hpp"
#include "Trackerboy.hpp"

#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QToolBar>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(Trackerboy &trackerboy);
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

    // config changes
    void onSoundChange();

    // statusbar
    void statusSetInstrument(int index);
    void statusSetWaveform(int index);
    void statusSetOctave(int octave);

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

    Trackerboy &mApp;
    
    WaveEditor *mWaveEditor;
    InstrumentEditor *mInstrumentEditor;
    ConfigDialog *mConfigDialog;

    // file name of the currently open file or "Untitled" for a new file
    QString mFilename;
    QString mDocumentName;

    // qwidgets not set in the ui file

    QToolBar *mSongToolbar = nullptr;
    QComboBox *mSongCombo = nullptr;

    // dock widgets
    QDockWidget *mDockInstruments = nullptr;
    QDockWidget *mDockWaveforms = nullptr;
    QDockWidget *mDockSongs = nullptr;
    QDockWidget *mDockSongProperties = nullptr;
    QDockWidget *mDockModuleProperties = nullptr;
    QDockWidget *mDockOrders = nullptr;


    // statusbar widgets
    QLabel *mStatusInstrument = nullptr;
    QLabel *mStatusWaveform = nullptr;
    QLabel *mStatusOctave = nullptr;
    QLabel *mStatusFramerate = nullptr;
    QLabel *mStatusSpeed = nullptr;
    QLabel *mStatusTempo = nullptr;
    QLabel *mStatusElapsed = nullptr;
    QLabel *mStatusPos = nullptr;
    QLabel *mSamplerateLabel = nullptr;

};
