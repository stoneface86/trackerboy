
#pragma once

#include "core/SyncWorker.hpp"
#include "core/model/ModuleDocument.hpp"
#include "core/model/ModuleModel.hpp"
#include "forms/AudioDiagDialog.hpp"
#include "forms/ConfigDialog.hpp"
#include "widgets/docks/ModuleSettingsWidget.hpp"
#include "widgets/editors/InstrumentEditor.hpp"
#include "widgets/editors/WaveEditor.hpp"
#include "widgets/OrderEditor.hpp"
#include "widgets/PatternEditor.hpp"
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
#include <QSplitter>
#include <QTreeView>

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
    void showAudioDiag();
    void showConfigDialog();

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

    void settingsMessageBox(QMessageBox &msgbox);

    // document management

    void addDocument(ModuleDocument *doc);

    bool saveDocument(ModuleDocument *doc);

    bool saveDocumentAs(ModuleDocument *doc);

    bool closeDocument(ModuleDocument *doc);

    void updateWindowTitle();

    //Trackerboy &mApp;
    Miniaudio &mMiniaudio;
    Config mConfig;

    PianoInput mPianoInput;


    // counter for how many times a new document has been created
    unsigned mDocumentCounter;

    ModuleModel mBrowserModel;

    bool mErrorSinceLastConfig;

    // dialogs
    AudioDiagDialog *mAudioDiag;
    ConfigDialog *mConfigDialog;


    // toolbars
    QToolBar mToolbarFile;
    QToolBar mToolbarEdit;
    QToolBar mToolbarTracker;


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
                QHBoxLayout mEditorLayout;
                    OrderEditor mOrderEditor;
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

    // File
    QMenu mMenuFile;
    QAction mActionFileNew;
    QAction mActionFileOpen;
    QAction mActionFileSave;
    QAction mActionFileSaveAs;
    QAction mActionFileClose;
    QAction mActionFileCloseAll;
    QAction mActionFileConfig;
    QAction mActionFileQuit;

    // Edit
    QMenu mMenuEdit;
    QAction *mActionEditUndo;
    QAction *mActionEditRedo;
    
    // View
    QMenu mMenuView;
    QMenu mMenuViewToolbars;
    QAction mActionViewResetLayout;

    // Tracker
    QMenu mMenuTracker;
    QAction mActionTrackerPlay;
    QAction mActionTrackerRestart;
    QAction mActionTrackerStop;
    QAction mActionTrackerToggleChannel;
    QAction mActionTrackerSolo;
    QAction mActionTrackerKill;

    // Window
    QMenu mMenuWindow;
    QAction mActionWindowPrev;
    QAction mActionWindowNext;

    // Help
    QMenu mMenuHelp;
    QAction mActionAudioDiag;
    QAction mActionHelpAboutQt;
    QAction mActionHelpAbout;

    // workers / threading

    Renderer *mRenderer;
    QThread mRenderThread;
    
    SyncWorker *mSyncWorker;
    QThread mSyncWorkerThread;


};
