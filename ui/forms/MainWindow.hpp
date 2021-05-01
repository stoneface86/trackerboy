
#pragma once

#include "core/Trackerboy.hpp"
#include "core/SyncWorker.hpp"
#include "core/model/ModuleDocument.hpp"
#include "core/model/ModuleModel.hpp"
#include "forms/AudioDiagDialog.hpp"
#include "forms/ConfigDialog.hpp"
//#include "forms/InstrumentEditor.hpp"
//#include "forms/WaveEditor.hpp"
//#include "widgets/docks/ModulePropertiesWidget.hpp"
//#include "widgets/docks/OrderWidget.hpp"
//#include "widgets/docks/SongPropertiesWidget.hpp"
//#include "widgets/docks/SongWidget.hpp"
//#include "widgets/docks/TableForm.hpp"
#include "widgets/visualizers/AudioScope.hpp"
#include "widgets/visualizers/PeakMeter.hpp"
#include "widgets/ModuleWindow.hpp"
//#include "widgets/PatternEditor.hpp"

#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QMdiArea>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QToolBar>
#include <QUndoView>
#include <QSplitter>
#include <QTreeView>

#include <vector>

//
// Main form for the application
//
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
    void showConfigDialog();

    // statusbar
    void trackerPositionChanged(QPoint const pos);

    void onAudioStart();
    void onAudioError();
    void onAudioStop();

    void onSubWindowActivated(QMdiSubWindow *window);
    void onDocumentClosed(ModuleDocument *doc);

    void updateWindowMenu();

private:
    Q_DISABLE_COPY(MainWindow)

    ModuleWindow* currentModuleWindow();

    void setupUi();

    void initState();

    void setupWindowMenu(QMenu &menu);

    void settingsMessageBox(QMessageBox &msgbox);

    void addDocument(ModuleDocument *doc);

    Trackerboy &mApp;

    // counter for how many times a new document has been created
    unsigned mDocumentCounter;
    ModuleDocument *mCurrentDocument;

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
    //QDockWidget mDockHistory;
    //    QUndoView mUndoView;

    // central widget (must be heap-alloc'd)
    QSplitter *mHSplitter;
        QTreeView mBrowser;
        QWidget mMainWidget;
            QVBoxLayout mMainLayout;
                QHBoxLayout mVisualizerLayout;
                    AudioScope mLeftScope;
                    PeakMeter mPeakMeter;
                    AudioScope mRightScope;
                QMdiArea mMdi;


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

    // Edit (created by PatternEditor)
    QMenu mMenuEdit;
    QAction *mActionEditUndo;
    QAction *mActionEditRedo;

    // Tracker
    QMenu mMenuTracker;
    QAction mActionTrackerPlay;
    QAction mActionTrackerRestart;
    QAction mActionTrackerStop;
    QAction mActionTrackerToggleChannel;
    QAction mActionTrackerSolo;

    // Window
    QMenu mMenuWindow;
    QMenu mMenuWindowToolbars;
    QAction mActionWindowResetLayout;
    QAction mActionWindowPrev;
    QAction mActionWindowNext;

    // Help
    QMenu mMenuHelp;
    QAction mActionAudioDiag;
    QAction mActionHelpAboutQt;
    QAction mActionHelpAbout;

    SyncWorker mSyncWorker;
    QThread mSyncWorkerThread;

};
