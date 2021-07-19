
#pragma once

#include "core/midi/IMidiReceiver.hpp"
#include "core/midi/Midi.hpp"
#include "core/model/InstrumentListModel.hpp"
#include "core/model/OrderModel.hpp"
#include "core/model/SongModel.hpp"
#include "core/model/PatternModel.hpp"
#include "core/model/WaveListModel.hpp"
#include "core/model/InstrumentChoiceModel.hpp"
#include "core/Module.hpp"
#include "core/ModuleFile.hpp"
#include "forms/AboutDialog.hpp"
#include "forms/AudioDiagDialog.hpp"
#include "forms/ConfigDialog.hpp"
//#include "widgets/editors/InstrumentEditor.hpp"
//#include "widgets/editors/WaveEditor.hpp"
//#include "widgets/PatternEditor.hpp"
#include "widgets/Sidebar.hpp"

#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMdiArea>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QToolBar>
#include <QTabBar>
#include <QUndoView>
#include <QShortcut>
#include <QSplitter>
#include <QTreeView>

#include <array>

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

private:
    Q_DISABLE_COPY(MainWindow)

    enum class PlayingStatusText {
        ready,
        playing,
        error
    };

    QToolBar* makeToolbar(QString const& title, QString const& objname);

    // implementation in MainWindow/actions.cpp -------------------------------

    void createActions();

    void setupViewMenu(QMenu *menu);

    void setupSongMenu(QMenu *menu);
    
    // ------------------------------------------------------------------------

    bool maybeSave();

    void setupUi();

    void initState();


    //
    // Shows and adds a "Change settings" button that opens the configuration
    // to the given QMessageBox
    //
    void settingsMessageBox(QMessageBox &msgbox);

    void updateWindowTitle();

    void updateOrderActions();

    void setPlayingStatus(PlayingStatusText type);

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
    OrderModel *mOrderModel;
    SongModel *mSongModel;
    PatternModel *mPatternModel;
    WaveListModel *mWaveModel;

    //Renderer mRenderer;

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
        QSpinBox *mOctaveSpin;
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

    //QDockWidget mDockInstrumentEditor;
    //    InstrumentEditor mInstrumentEditor;

    //QDockWidget mDockWaveformEditor;
    //    WaveEditor mWaveEditor;

    QDockWidget mDockHistory;
        QUndoView mUndoView;

    // widgets
    Sidebar *mSidebar;
    //PatternEditor *mPatternEditor;

    // statusbar widgets
    QLabel mStatusRenderer;
    QLabel mStatusSpeed;
    QLabel mStatusTempo;
    QLabel mStatusElapsed;
    QLabel mStatusPos;
    QLabel mStatusSamplerate;

    //QShortcut mPlayAndStopShortcut;


};
