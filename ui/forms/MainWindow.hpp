
#pragma once

#include "core/audio/Renderer.hpp"
#include "core/midi/IMidiReceiver.hpp"
#include "core/midi/Midi.hpp"
#include "core/model/InstrumentListModel.hpp"
#include "core/model/SongModel.hpp"
#include "core/model/PatternModel.hpp"
#include "core/model/WaveListModel.hpp"
#include "core/Config.hpp"
#include "core/Module.hpp"
#include "core/ModuleFile.hpp"
#include "core/PianoInput.hpp"
#include "forms/AboutDialog.hpp"
#include "forms/AudioDiagDialog.hpp"
#include "forms/ConfigDialog.hpp"
//#include "widgets/editors/InstrumentEditor.hpp"
//#include "widgets/editors/WaveEditor.hpp"
#include "misc/TableActions.hpp"
#include "widgets/PatternEditor.hpp"
#include "widgets/Sidebar.hpp"

#include "trackerboy/engine/Frame.hpp"

#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QToolBar>
#include <QSpinBox>

//
// Main form for the application
//
class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow();
    virtual ~MainWindow();

    QMenu* createPopupMenu() override;

protected:

    virtual void closeEvent(QCloseEvent *evt) override;

    virtual void showEvent(QShowEvent *evt) override;

private slots:

    // implementation in MainWindow/slots.cpp - BEGIN -------------------------

    // actions
    void onFileNew();
    void onFileOpen();
    bool onFileSave();
    bool onFileSaveAs();

    void onSongOrderInsert();
    void onSongOrderRemove();
    void onSongOrderDuplicate();
    void onSongOrderMoveUp();
    void onSongOrderMoveDown();

    void onTrackerPlay();
    void onTrackerPlayAtStart();
    void onTrackerPlayFromCursor();
    void onTrackerStep();
    void onTrackerStop();
    void onTrackerSolo();
    void onTrackerToggleOutput();
    void onTrackerKill();
    
    void onViewResetLayout();

    // config changes
    void onConfigApplied(Config::Categories categories);

    // dialog show slots (lazy loading)
    void showAboutDialog();
    void showAudioDiag();
    void showConfigDialog();
    void showExportWavDialog();

    void onAudioStart();
    void onAudioError();
    void onAudioStop();
    void onFrameSync();

    // shortcut slots
    void previousInstrument();
    void nextInstrument();
    void previousPattern();
    void nextPattern();
    void increaseOctave();
    void decreaseOctave();
    void playOrStop();

    // misc slots
    void onPreviewNote();

    // implementation in MainWindow/slots.cpp - END ---------------------------

private:
    Q_DISABLE_COPY(MainWindow)

    enum class PlayingStatusText {
        ready,
        playing,
        error
    };

    QDockWidget* makeDock(QString const& title, QString const& objname);

    //
    // Creates a new toolbar with the given title and object name. This class
    // takes ownership of the toolbar.
    //
    QToolBar* makeToolbar(QString const& title, QString const& objname);

    //
    // Utility function for tracker play slots. Checks if the renderer is
    // stepping and if so, steps out. Does nothing otherwise. true is returned
    // if the renderer was stepped out, false otherwise.
    //
    bool checkAndStepOut();

    // implementation in MainWindow/actions.cpp - BEGIN -----------------------

    //
    // Creates all actions used by the application
    //
    void createActions(TableActions const& instrumentActions, TableActions const& waveformActions);

    //
    // Setups the View menu in the given QMenu instance.
    //
    void setupViewMenu(QMenu *menu);

    //
    // Adds order actions to the given menu
    //
    void setupSongMenu(QMenu *menu);

    //
    // setups the menu with actions for table operations using the given model
    //
    void setupTableMenu(QMenu *menu, TableActions const& actions);

    // implementation in MainWindow/actions.cpp - END -------------------------

    //
    // Prompts the user to save if the module is modified, do nothing otherwise
    // To be called before saving, loading and when closing the application. If
    // this function returns false, do not continue with the save/load or close.
    // true is returned if:
    //  * The user was prompted to save, saved, and the save was successful
    //  * The user was prompted to save but chose to discard changes
    //  * The document was not modified, so the user was not prompted to save
    // false is returned for all other cases
    //
    bool maybeSave();

    void commitModels();

    //
    // Setups the UI, should only be called once and by the constructor
    //
    void setupUi();

    //
    // Resets all toolbars and docks to the initial state.
    //
    void initState();

    //
    // Shows and adds a "Change settings" button that opens the configuration
    // to the given QMessageBox
    //
    void settingsMessageBox(QMessageBox &msgbox);

    //
    // Sets the window title using the current filename
    //
    void updateWindowTitle();

    //
    // Updates action enabled state for order actions
    //
    void updateOrderActions();

    //
    // Sets the playing status text in statusbar
    //  PlayingStatusText::playing - "Playing"
    //  PlayingStatusText::ready - "Ready"
    //  PlayingStatusText::error - "Device error"
    //
    void setPlayingStatus(PlayingStatusText type);

    //
    // Updates the current midi receiver based on the newWidget that
    // recieved focus.
    //
    void handleFocusChange(QWidget *oldWidget, QWidget *newWidget);

    //
    // Shows a message and disables the configured midi device.
    // If causedByError is true, then the messagebox states it was caused by
    // an error, otherwise it states that it failed to initialize the device.
    //
    void disableMidi(bool causedByError);

    QString const mUntitledString;

    #ifdef QT_DEBUG
    bool mSaveConfig = true;
    #endif

    Config mConfig;

    Midi mMidi;
    IMidiReceiver *mMidiReceiver;
    bool mMidiNoteDown;

    PianoInput mPianoInput;

    Module *mModule;
    ModuleFile mModuleFile;

    InstrumentListModel *mInstrumentModel;
    SongModel *mSongModel;
    PatternModel *mPatternModel;
    WaveListModel *mWaveModel;

    Renderer *mRenderer;

    bool mErrorSinceLastConfig;
    trackerboy::Frame mLastEngineFrame;
    int mFrameSkip;

    // dialogs
    AboutDialog *mAboutDialog;
    AudioDiagDialog *mAudioDiag;
    ConfigDialog *mConfigDialog;


    // toolbars
    QToolBar *mToolbarFile;
    QToolBar *mToolbarEdit;
    QToolBar *mToolbarSong;
    QToolBar *mToolbarTracker;
    QToolBar *mToolbarInput;
    QToolBar *mToolbarInstrument;
        QComboBox *mInstrumentCombo;

    // actions shared by multiple menus (parented by MainWindow)
    QAction *mActionOrderInsert;
    QAction *mActionOrderRemove;
    QAction *mActionOrderDuplicate;
    QAction *mActionOrderMoveUp;
    QAction *mActionOrderMoveDown;

    QAction *mActionViewReset;

    QAction *mActionFollowMode;

    QMenu *mSongOrderContextMenu;

    // dock widgets

    //QDockWidget *mDockInstrumentEditor;
    //QDockWidget *mDockWaveformEditor;
    QDockWidget *mDockInstruments;
    QDockWidget *mDockWaveforms;
    QDockWidget *mDockHistory;

    // widgets
    Sidebar *mSidebar;
    PatternEditor *mPatternEditor;

    // statusbar widgets
    QLabel *mStatusRenderer;
    QLabel *mStatusSpeed;
    QLabel *mStatusTempo;
    QLabel *mStatusElapsed;
    QLabel *mStatusPos;
    QLabel *mStatusSamplerate;


};
