
#pragma once

#include "core/SyncWorker.hpp"
#include "core/model/ModuleDocument.hpp"
#include "core/model/ModuleModel.hpp"
#include "core/model/InstrumentChoiceModel.hpp"
#include "forms/AboutDialog.hpp"
#include "forms/AudioDiagDialog.hpp"
#include "forms/ConfigDialog.hpp"
#include "widgets/docks/ModuleSettingsWidget.hpp"
#include "widgets/editors/InstrumentEditor.hpp"
#include "widgets/editors/WaveEditor.hpp"
#include "widgets/OrderEditor.hpp"
#include "widgets/PatternEditor.hpp"
#include "widgets/Sidebar.hpp"
#include "widgets/visualizers/AudioScope.hpp"
#include "widgets/visualizers/PeakMeter.hpp"

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
    explicit MainWindow(Miniaudio &miniaudio);
    ~MainWindow();

    QMenu* createPopupMenu() override;

protected:

    void closeEvent(QCloseEvent *evt) override;

    void showEvent(QShowEvent *evt) override;

private slots:

    // actions
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onFileClose();
    void onFileCloseAll();
    
    void onViewResetLayout();

    void onWindowNext();
    void onWindowPrevious();

    // config changes
    void onConfigApplied(Config::Categories categories);

    // dialog show slots (lazy loading)
    void showAboutDialog();
    void showAudioDiag();
    void showConfigDialog();
    void showExportWavDialog();

    // statusbar
    void trackerPositionChanged(QPoint const pos);

    void onAudioStart();
    void onAudioError();
    void onAudioStop();

    void onTabChanged(int tabIndex);

    void onBrowserDoubleClick(QModelIndex const& index);

    void onDocumentModified(bool modified);

    void updateWindowMenu();

    void closeTab(int index);

private:
    Q_DISABLE_COPY(MainWindow)

    void setupUi();

    void initState();

    void setupViewMenu(QMenu &menu);

    void setupSongMenu(QMenu &menu);

    void settingsMessageBox(QMessageBox &msgbox);

    // document management

    void addDocument(ModuleDocument *doc);

    bool saveDocument(ModuleDocument *doc);

    bool saveDocumentAs(ModuleDocument *doc);

    bool closeDocument(ModuleDocument *doc);

    void updateWindowTitle();

    void updateOrderActions();

    Miniaudio &mMiniaudio;
    Config mConfig;

    PianoInput mPianoInput;


    // counter for how many times a new document has been created
    int mDocumentCounter;

    ModuleModel mBrowserModel;

    bool mErrorSinceLastConfig;

    // dialogs
    AboutDialog *mAboutDialog;
    AudioDiagDialog *mAudioDiag;
    ConfigDialog *mConfigDialog;


    // toolbars
    QToolBar mToolbarFile;
    QToolBar mToolbarEdit;
    QToolBar mToolbarSong;
    QToolBar mToolbarTracker;
    QToolBar mToolbarInput;
        QLabel mOctaveLabel;
        QSpinBox mOctaveSpin;
        QLabel mEditStepLabel;
        QSpinBox mEditStepSpin;
    QToolBar mToolbarInstrument;
        QComboBox mInstrumentCombo;
        InstrumentChoiceModel mInstrumentChoiceModel;


    // dock widgets
    QDockWidget mDockModuleSettings;
        ModuleSettingsWidget mModuleSettingsWidget;

    QDockWidget mDockInstrumentEditor;
        InstrumentEditor mInstrumentEditor;

    QDockWidget mDockWaveformEditor;
        WaveEditor mWaveEditor;

    QDockWidget mDockHistory;
        QUndoView mUndoView;

    // central widget (must be heap-alloc'd)
    QSplitter *mHSplitter;
        QTreeView mBrowser;
        QWidget mMainWidget;
            QVBoxLayout mMainLayout;
                //QHBoxLayout mVisualizerLayout;
                //    AudioScope mLeftScope;
                //    PeakMeter mPeakMeter;
                //    AudioScope mRightScope;
                QTabBar mTabs;
                QWidget mEditorWidget;
                    QHBoxLayout mEditorLayout;
                        // QVBoxLayout mGroupLayout;
                        //     QGroupBox mOrderGroup;
                        //         QGridLayout mOrderLayout;
                        //             OrderEditor mOrderEditor;
                        //     QGroupBox mSongGroup;
                        //         //QGridLayout mSongLayout;
                        Sidebar mSidebar;
                        PatternEditor mPatternEditor;


    // statusbar widgets
    QLabel mStatusRenderer;
    QLabel mStatusFramerate;
    QLabel mStatusSpeed;
    QLabel mStatusTempo;
    QLabel mStatusElapsed;
    QLabel mStatusPos;
    QLabel mStatusSamplerate;

    // menus

    // action indices
    enum ActionIndex {
        ActionFileNew,
        ActionFileOpen,
        ActionFileConfig,
        ActionFileQuit,
        ActionViewResetLayout,
        ActionHelpAudioDiag,
        ActionHelpAbout,
        ActionHelpAboutQt,

        // document actions - these actions require an open document
        // when there are no open documents they are disabled
        DOCUMENT_ACTIONS_BEGIN,

        ActionFileSave = DOCUMENT_ACTIONS_BEGIN,
        ActionFileSaveAs,
        ActionFileExportWav,
        ActionFileClose,
        ActionFileCloseAll,

        ActionEditCopy,
        ActionEditCut,
        ActionEditPaste,
        ActionEditPasteMix,
        ActionEditErase,
        ActionEditSelectAll,
        ActionEditNoteIncrease,
        ActionEditNoteDecrease,
        ActionEditOctaveIncrease,
        ActionEditOctaveDecrease,
        ActionEditTranspose,
        ActionEditReverse,
        ActionEditKeyRepetition,

        ActionSongOrderInsert,
        ActionSongOrderRemove,
        ActionSongOrderDuplicate,
        ActionSongOrderMoveUp,
        ActionSongOrderMoveDown,

        ActionTrackerPlay,
        ActionTrackerRestart,
        ActionTrackerPlayCurrentRow,
        ActionTrackerStepRow,
        ActionTrackerStop,
        ActionTrackerRecord,
        ActionTrackerToggleChannel,
        ActionTrackerSolo,
        ActionTrackerKill,
        ActionTrackerRepeat,
        ActionTrackerFollow,

        ActionWindowPrev,
        ActionWindowNext,

        ACTION_COUNT
    };

    std::array<QAction, ACTION_COUNT> mActions;

    // File
    QMenu mMenuFile;

    // Edit
    QMenu mMenuEdit;
        QAction *mActionEditUndo;
        QAction *mActionEditRedo;
        QMenu mMenuTranspose;
    
    // Song
    QMenu mMenuSong;

    // View
    QMenu mMenuView;
        QMenu mMenuViewToolbars;

    // Tracker
    QMenu mMenuTracker;

    // Window
    QMenu mMenuWindow;

    // Help
    QMenu mMenuHelp;

    QMenu mContextMenuOrder;

    QShortcut mPlayAndStopShortcut;


    // workers / threading

    Renderer *mRenderer;
    QThread mRenderThread;
    
    SyncWorker *mSyncWorker;
    QThread mSyncWorkerThread;


};
