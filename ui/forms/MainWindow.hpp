
#pragma once

#include "core/Trackerboy.hpp"
#include "forms/AudioDiagDialog.hpp"
#include "forms/ConfigDialog.hpp"
#include "forms/InstrumentEditor.hpp"
#include "forms/WaveEditor.hpp"
#include "widgets/docks/ModulePropertiesWidget.hpp"
#include "widgets/docks/OrderWidget.hpp"
#include "widgets/docks/SongPropertiesWidget.hpp"
#include "widgets/docks/SongWidget.hpp"
#include "widgets/docks/TableForm.hpp"
#include "widgets/docks/Visualizer.hpp"
#include "widgets/PatternEditor.hpp"

#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QTimer>
#include <QToolBar>


class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(Trackerboy &trackerboy);
    ~MainWindow();

    QMenu* createPopupMenu() override;

protected:

    void closeEvent(QCloseEvent *evt) override;

    void showEvent(QShowEvent *evt) override;

private slots:
    void updateWindowTitle();

    // actions
    void onFileNew();
    void onFileOpen();
    bool onFileSave();
    bool onFileSaveAs();

    void onWindowResetLayout();

    // config changes
    void onConfigApplied(Config::Categories categories);

    // dialog show slots (lazy loading)
    void showAudioDiag();
    void showInstrumentEditor();
    void showWaveEditor();
    void showConfigDialog();

    // statusbar
    void statusSetInstrument(int index);
    void statusSetWaveform(int index);
    void statusSetOctave(int octave);

    void audioReturn();

private:

    //static void setupAction(QAction &action, const char *text, const char *tooltip, QKeySequence const &seq = QKeySequence());

    // To be called before loading a new document. Prompts user to save if the
    // current document is modified. Returns false if the user does not want to continue
    bool maybeSave();

    void setFilename(QString filename);

    // enable or disable all models
    void setModelsEnabled(bool enabled);

    void setupUi();

    void initState();

    void setupWindowMenu(QMenu &menu);


    Trackerboy &mApp;
    
    
    // file name of the currently open file or "Untitled" for a new file
    QString mFilename;
    QString mDocumentName;

    QTimer mReturnTimer;

    // dialogs
    AudioDiagDialog *mAudioDiag;
    ConfigDialog *mConfigDialog;
    InstrumentEditor *mInstrumentEditor;
    WaveEditor *mWaveEditor;
    QFileDialog mModuleFileDialog;


    // toolbars
    QToolBar mToolbarFile;
    QToolBar mToolbarEdit;
    QToolBar mToolbarTracker;
    QToolBar mToolbarSong;
        QComboBox mSongCombo;

    // dock widgets
    QDockWidget mDockInstruments;
        TableForm mInstrumentWidget;

    QDockWidget mDockWaveforms;
        TableForm mWaveformWidget;

    QDockWidget mDockSongs;
        SongWidget mSongWidget;

    QDockWidget mDockSongProperties;
        SongPropertiesWidget mSongPropertiesWidget;

    QDockWidget mDockModuleProperties;
        ModulePropertiesWidget mModulePropertiesWidget;

    QDockWidget mDockOrders;
        OrderWidget mOrderWidget;

    QDockWidget mDockVisualizer;
        Visualizer mVisualizer;

    // central widget (must be heap-alloc'd)
    PatternEditor *mPatternEditor;


    // statusbar widgets
    QLabel mStatusInstrument;
    QLabel mStatusWaveform;
    QLabel mStatusOctave;
    QLabel mStatusFramerate;
    QLabel mStatusSpeed;
    QLabel mStatusTempo;
    QLabel mStatusElapsed;
    QLabel mStatusPos;
    QLabel mStatusSamplerate;

    // menus

    // File
    QMenu mMenuFile;
    QAction mActionFileNew;
    QAction mActionFileOpen;
    QAction mActionFileSave;
    QAction mActionFileSaveAs;
    QAction mActionFileConfig;
    QAction mActionFileQuit;

    // Edit (created by PatternEditor)
    QMenu mMenuEdit;

    // Song (created by SongWidget)
    QMenu mMenuSong;
    QAction mActionSongPrev;
    QAction mActionSongNext;

    // Order (created by OrderWidget)
    QMenu mMenuOrder;

    // Instrument (created by TableForm)
    QMenu mMenuInstrument;

    // Waveform (created by TableForm)
    QMenu mMenuWaveform;

    // Tracker
    QMenu mMenuTracker;
    QAction mActionTrackerPlay;
    QAction mActionTrackerPlayPattern;
    QAction mActionTrackerPlayStart;
    QAction mActionTrackerPlayCursor;
    QAction mActionTrackerStop;
    QAction mActionTrackerEditMode;
    QAction mActionTrackerToggleChannel;
    QAction mActionTrackerSolo;

    // Window
    QMenu mMenuWindow;
    QMenu mMenuWindowToolbars;
    QAction mActionWindowResetLayout;

    // Help
    QMenu mMenuHelp;
    QAction mActionAudioDiag;
    QAction mActionHelpAboutQt;
    QAction mActionHelpAbout;
};
