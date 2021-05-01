
#include "MainWindow.hpp"

#include "core/samplerates.hpp"
#include "misc/IconManager.hpp"
#include "misc/utils.hpp"
#include "widgets/ModuleWindow.hpp"

#include <QApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QScreen>
#include <QMenuBar>
#include <QStatusBar>
#include <QMdiSubWindow>
#include <QtDebug>

#include <array>


#define setObjectNameFromDeclared(var) var.setObjectName(QStringLiteral(#var))
#define connectActionToThis(action, slot) connect(&action, &QAction::triggered, this, &MainWindow::slot)

constexpr int TOOLBAR_ICON_WIDTH = 16;
constexpr int TOOLBAR_ICON_HEIGHT = 16;

MainWindow::MainWindow(Trackerboy &trackerboy) :
    QMainWindow(),
    mApp(trackerboy),
    mDocumentCounter(0),
    mCurrentDocument(nullptr),
    mErrorSinceLastConfig(false),
    mAudioDiag(nullptr),
    mConfigDialog(nullptr),
    mToolbarFile(),
    mToolbarEdit(),
    mToolbarTracker(),
    mSyncWorker(mApp.renderer, mLeftScope, mRightScope),
    mSyncWorkerThread()

{
    setupUi();

    // read in application configuration
    mApp.config.readSettings();
    // apply the read in configuration
    onConfigApplied(Config::CategoryAll);

    setWindowIcon(IconManager::getAppIcon());

    QSettings settings;

    // restore geomtry from the last session
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();

    #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    if (geometry.isEmpty()) {
        // initialize window size to 3/4 of the screen's width and height
        // screen() was added at version 5.14
        const QRect availableGeometry = window()->screen()->availableGeometry();
        resize(availableGeometry.width() / 4 * 3, availableGeometry.height() / 4 * 3);
        move((availableGeometry.width() - width()) / 2,
            (availableGeometry.height() - height()) / 2);

    } else {
        #else
    if (!geometry.isEmpty()) {
        #endif
        restoreGeometry(geometry);
    }

    // restore window state if it exists
    const QByteArray windowState = settings.value("windowState").toByteArray();
    if (windowState.isEmpty()) {
        // default layout
        initState();
    } else {
        addToolBar(&mToolbarFile);
        addToolBar(&mToolbarEdit);
        addToolBar(&mToolbarTracker);
        restoreState(windowState);
    }

    // window title
    setWindowTitle(tr("Trackerboy"));

    // audio sync worker thread
    mSyncWorker.moveToThread(&mSyncWorkerThread);
    mSyncWorkerThread.setObjectName(QStringLiteral("sync worker thread"));
    mSyncWorkerThread.start();
}

MainWindow::~MainWindow() {
    mSyncWorkerThread.quit();
    mSyncWorkerThread.wait();
}

QMenu* MainWindow::createPopupMenu() {
    // we can't return a reference to mMenuWindow as QMainWindow will delete it
    // a new menu must be created
    auto menu = new QMenu(this);
    setupWindowMenu(*menu);
    return menu;
}

void MainWindow::closeEvent(QCloseEvent *evt) {

    mMdi.closeAllSubWindows();

    if (mMdi.currentSubWindow() == nullptr) {
        // all modules closed, accept this event and close
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        evt->accept();
    } else {
        // user canceled closing a document, ignore this event
        evt->ignore();
    }
    
}

void MainWindow::showEvent(QShowEvent *evt) {
    Q_UNUSED(evt);
}

// SLOTS ---------------------------------------------------------------------

// action slots

void MainWindow::onFileNew() {

    ++mDocumentCounter;
    auto doc = new ModuleDocument(this);
    QString name = tr("Untitled %1").arg(mDocumentCounter);
    doc->setName(name);
    addDocument(doc);

}

void MainWindow::onFileOpen() {
    auto path = QFileDialog::getOpenFileName(
        this,
        tr("Open module"),
        "",
        tr(ModuleWindow::MODULE_FILE_FILTER)
    );

    if (path.isEmpty()) {
        return;
    }

    // make sure the document isn't already open
    QFileInfo pathInfo(path);
    for (auto doc : mBrowserModel.documents()) {
        if (doc->hasFile()) {
            QFileInfo docInfo(doc->filepath());
            if (pathInfo == docInfo) {
                for (auto win : mMdi.subWindowList()) {
                    if (static_cast<ModuleWindow*>(win->widget())->document() == doc) {
                        mMdi.setActiveSubWindow(win);
                        break;
                    }
                }
                return; // document is already open, don't open it again
            }
        }
    }

    auto *doc = new ModuleDocument(path, this);

    if (doc) {
        if (doc->lastError() == trackerboy::FormatError::none) {
            addDocument(doc);
        } else {
            // TODO: report error to user
            delete doc;
        }
    }

}

bool MainWindow::onFileSave() {
    return currentModuleWindow()->save();
}

bool MainWindow::onFileSaveAs() {
    return currentModuleWindow()->saveAs();
}

void MainWindow::onWindowResetLayout() {
    // remove everything
    removeToolBar(&mToolbarFile);
    removeToolBar(&mToolbarEdit);
    removeToolBar(&mToolbarTracker);

    initState();
}

void MainWindow::onConfigApplied(Config::Categories categories) {
    if (categories.testFlag(Config::CategorySound)) {
        auto &sound = mApp.config.sound();
        auto samplerate = SAMPLERATE_TABLE[sound.samplerateIndex];
        mStatusSamplerate.setText(tr("%1 Hz").arg(samplerate));

        auto samplesPerFrame = samplerate / 60;
        mSyncWorker.setSamplesPerFrame(samplesPerFrame);
        mLeftScope.setDuration(samplesPerFrame);
        mRightScope.setDuration(samplesPerFrame);

        mApp.renderer.setConfig(sound);
        mErrorSinceLastConfig = mApp.renderer.lastDeviceError() != MA_SUCCESS;
        if (isVisible() && mErrorSinceLastConfig) {
            QMessageBox msgbox(this);
            msgbox.setIcon(QMessageBox::Critical);
            msgbox.setText(tr("Could not initialize device"));
            msgbox.setInformativeText(tr("The configured device could not be initialized. Playback is disabled."));
            settingsMessageBox(msgbox);
        }
    }

    if (categories.testFlag(Config::CategoryAppearance)) {
        auto &appearance = mApp.config.appearance();

        setStyleSheet(QStringLiteral(R"stylesheet(
PatternEditor PatternGrid {
    font-family: %5;
    font-size: %6pt;
}

AudioScope {
    background-color: %1;
    color: %3;
}

OrderWidget QTableView {
    background-color: %1;
    gridline-color: %2;
    color: %3;
    selection-color: %3;
    selection-background-color: %4;
    font-family: %5;
}

OrderWidget QTableView QTableCornerButton::section {
    background-color: %1;
    border-right: 1px solid %2;
    border-bottom: 1px solid %2;
    border-top: none;
    border bottom: none;
}

OrderWidget QTableView QHeaderView {
    background-color: %1;
    color: %3;
    font-family: %5;
}

OrderWidget QTableView QHeaderView::section {
    background-color: %1;
    border-right: 1px solid %2;
    border-bottom: 1px solid %2;
    border-top: none;
    border bottom: none;
}

)stylesheet").arg(
        appearance.colors[+Color::background].name(),
        appearance.colors[+Color::line].name(),
        appearance.colors[+Color::foreground].name(),
        appearance.colors[+Color::selection].name(),
        appearance.font.family(),
        QString::number(appearance.font.pointSize())
        ));
    }

    //if (categories.testFlag(Config::CategoryKeyboard)) {
    //    mPianoInput = mApp.config.keyboard().pianoInput;
    //}

    mApp.config.writeSettings();
}

void MainWindow::showAudioDiag() {
    if (mAudioDiag == nullptr) {
        mAudioDiag = new AudioDiagDialog(mApp.renderer, this);
    }

    mAudioDiag->show();
}

void MainWindow::showConfigDialog() {
    if (mConfigDialog == nullptr) {
        mConfigDialog = new ConfigDialog(mApp.config, this);
        mConfigDialog->resetControls();

        // configuration changed, apply settings
        connect(mConfigDialog, &ConfigDialog::applied, this, &MainWindow::onConfigApplied);
        for (auto subwin : mMdi.subWindowList()) {
            auto win = static_cast<ModuleWindow*>(subwin->widget());
            connect(mConfigDialog, &ConfigDialog::applied, win, &ModuleWindow::applyConfiguration);
        }
    }

    mConfigDialog->show();
}

void MainWindow::trackerPositionChanged(QPoint const pos) {
    auto pattern = pos.x();
    auto row = pos.y();
    
    // TODO: update tracker position for the module being rendered
    //auto &grid = mPatternEditor.grid();
    //grid.setTrackerCursor(row, pattern);

    mStatusPos.setText(QStringLiteral("%1 / %2").arg(pattern).arg(row));
}

void MainWindow::onAudioStart() {
    mStatusRenderer.setText(tr("Playing"));
}

void MainWindow::onAudioError() {
    mStatusRenderer.setText(tr("Device error"));
    if (!mErrorSinceLastConfig) {
        mErrorSinceLastConfig = true;
        QMessageBox msgbox(this);
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("Audio error"));
        msgbox.setInformativeText(tr(
            "A device error has occurred during playback.\n\n" \
            "Playback is disabled until a new device is configured in the settings."
        ));
        settingsMessageBox(msgbox);

    }
}

void MainWindow::onAudioStop() {
    if (!mErrorSinceLastConfig) {
        mStatusRenderer.setText(tr("Ready"));
    }
}

void MainWindow::onSubWindowActivated(QMdiSubWindow *window) {
    bool const hasWindow = window != nullptr;
    mActionFileSave.setEnabled(hasWindow);
    mActionFileSaveAs.setEnabled(hasWindow);
    mActionFileClose.setEnabled(hasWindow);
    mActionFileCloseAll.setEnabled(hasWindow);
    mActionWindowNext.setEnabled(hasWindow);
    mActionWindowPrev.setEnabled(hasWindow);

    auto doc = hasWindow
        ? static_cast<ModuleWindow*>(window->widget())->document() 
        : nullptr;

    if (mCurrentDocument != doc) {
        mCurrentDocument = doc;
    }

}

void MainWindow::onDocumentClosed(ModuleDocument *doc) {
    mBrowserModel.removeDocument(doc);
    
    // no longer using this document, delete it
    delete doc;
}

void MainWindow::updateWindowMenu() {
    mMenuWindow.clear();
    mMenuWindow.addMenu(&mMenuWindowToolbars);
    mMenuWindow.addAction(&mActionWindowResetLayout);
    mMenuWindow.addSeparator();
    mMenuWindow.addAction(&mActionWindowPrev);
    mMenuWindow.addAction(&mActionWindowNext);
    
    auto const windows = mMdi.subWindowList();
    if (windows.size() > 0) {
        mMenuWindow.addSeparator();

        int i = 1;
        for (auto window : windows) {

            auto moduleWin = static_cast<ModuleWindow*>(window->widget());
            
            auto textFmt = (i < 9) ? "&%1 %2" : "%1 %2";
            QString text = tr(textFmt).arg(i).arg(moduleWin->document()->name());

            auto action = mMenuWindow.addAction(text, window, [this, window]() {
                mMdi.setActiveSubWindow(window);
                });
            action->setCheckable(true);
            action->setChecked(window == mMdi.activeSubWindow());


            ++i;
        }
    }

}

// PRIVATE METHODS -----------------------------------------------------------

ModuleWindow* MainWindow::currentModuleWindow() {
    // static cast is used because all QMdiSubWindows in this application will have a ModuleWindow widget
    // qobject_cast is unneccessary
    return static_cast<ModuleWindow*>(mMdi.currentSubWindow()->widget());
}

void MainWindow::addDocument(ModuleDocument *doc) {
    // add the document to the model
    auto index = mBrowserModel.addDocument(doc);
    // expand the index of the newly added model
    mBrowser.expand(index);

    // create an Mdi subwindow for this document
    auto docWin = new ModuleWindow(mApp, doc);
    // apply configuration for the first time
    docWin->applyConfiguration(Config::CategoryAll);
    connect(docWin, &ModuleWindow::documentClosed, this, &MainWindow::onDocumentClosed);
    if (mConfigDialog) {
        connect(mConfigDialog, &ConfigDialog::applied, docWin, &ModuleWindow::applyConfiguration);
    }
    // add it the MDI area and show it
    mMdi.addSubWindow(docWin);
    docWin->show();

}

void MainWindow::setupUi() {

    //auto &undoStack = mApp.document.undoStack();

    // CENTRAL WIDGET ========================================================

    // MainWindow expects this to heap-alloc'd as it will manually delete the widget
    mHSplitter = new QSplitter(Qt::Horizontal, this);

    mMdi.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMdi.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMdi.setViewMode(QMdiArea::TabbedView);

    mBrowser.setModel(&mBrowserModel);
    mBrowser.setHeaderHidden(true);

    mVisualizerLayout.addWidget(&mLeftScope);
    mVisualizerLayout.addWidget(&mPeakMeter, 1);
    mVisualizerLayout.addWidget(&mRightScope);

    mMainLayout.addLayout(&mVisualizerLayout);
    mMainLayout.addWidget(&mMdi, 1);
    mMainLayout.setMargin(0);
    mMainWidget.setLayout(&mMainLayout);

    mHSplitter->addWidget(&mBrowser);
    mHSplitter->addWidget(&mMainWidget);
    mHSplitter->setStretchFactor(0, 0);
    mHSplitter->setStretchFactor(1, 1);
    setCentralWidget(mHSplitter);

    // ACTIONS ===============================================================

    setupAction(mActionFileNew, "&New", "Create a new module", Icons::fileNew, QKeySequence::New);
    setupAction(mActionFileOpen, "&Open", "Open an existing module", Icons::fileOpen, QKeySequence::Open);
    setupAction(mActionFileSave, "&Save", "Save the module", Icons::fileSave, QKeySequence::Save);
    setupAction(mActionFileSaveAs, "Save &As...", "Save the module to a new file", QKeySequence::SaveAs);
    setupAction(mActionFileClose, "Close", "Close the current module", QKeySequence::Close);
    setupAction(mActionFileCloseAll, "Close All", "Closes all open modules");
    setupAction(mActionFileConfig, "&Configuration...", "Change application settings", Icons::fileConfig);
    setupAction(mActionFileQuit, "&Quit", "Exit the application", QKeySequence::Quit);

    /*mActionEditUndo = undoStack.createUndoAction(this);
    mActionEditUndo->setIcon(IconManager::getIcon(Icons::editUndo));
    mActionEditUndo->setShortcut(QKeySequence::Undo);
    mActionEditRedo = undoStack.createRedoAction(this);
    mActionEditRedo->setIcon(IconManager::getIcon(Icons::editRedo));
    mActionEditRedo->setShortcut(QKeySequence::Redo);*/

    setupAction(mActionTrackerPlay, "&Play", "Resume playing or play the song from the current position", Icons::trackerPlay);
    setupAction(mActionTrackerRestart, "Play from start", "Begin playback of the song from the start", Icons::trackerRestart);
    setupAction(mActionTrackerStop, "&Stop", "Stop playing", Icons::trackerStop);
    setupAction(mActionTrackerToggleChannel, "Toggle channel output", "Enables/disables sound output for the current track");
    setupAction(mActionTrackerSolo, "Solo", "Solos the current track");

    setupAction(mActionWindowResetLayout, "Reset layout", "Rearranges all docks and toolbars to the default layout");
    setupAction(mActionWindowPrev, "Pre&vious", "Move the focus to the previous module");
    setupAction(mActionWindowNext, "Ne&xt", "Move the focus to the next module");


    setupAction(mActionAudioDiag, "Audio diagnostics...", "Shows the audio diagnostics dialog");
    setupAction(mActionHelpAbout, "&About", "About this program");
    setupAction(mActionHelpAboutQt, "About &Qt", "Shows information about Qt");

    // default action states
    mActionFileSave.setEnabled(false);
    mActionFileSaveAs.setEnabled(false);
    mActionFileClose.setEnabled(false);
    mActionFileCloseAll.setEnabled(false);

    // MENUS ==============================================================


    mMenuFile.setTitle(tr("&File"));
    mMenuFile.addAction(&mActionFileNew);
    mMenuFile.addAction(&mActionFileOpen);
    mMenuFile.addAction(&mActionFileSave);
    mMenuFile.addAction(&mActionFileSaveAs);
    mMenuFile.addAction(&mActionFileClose);
    mMenuFile.addAction(&mActionFileCloseAll);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileConfig);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileQuit);

    mMenuEdit.setTitle(tr("&Edit"));
    //mMenuEdit.addAction(mActionEditUndo);
    //mMenuEdit.addAction(mActionEditRedo);
    //mMenuEdit.addSeparator();
    //mPatternEditor.setupMenu(mMenuEdit);

    mMenuTracker.setTitle(tr("&Tracker"));
    mMenuTracker.addAction(&mActionTrackerPlay);
    mMenuTracker.addAction(&mActionTrackerRestart);
    mMenuTracker.addAction(&mActionTrackerStop);
    mMenuTracker.addSeparator();

    /*{
        auto &actions = mPatternEditor.trackerActions();
        mMenuTracker.addAction(&actions.play);
        mMenuTracker.addAction(&actions.restart);
        mMenuTracker.addAction(&actions.playRow);
        mMenuTracker.addAction(&actions.record);

    }*/

    mMenuTracker.addSeparator();
    mMenuTracker.addAction(&mActionTrackerToggleChannel);
    mMenuTracker.addAction(&mActionTrackerSolo);

    mMenuWindow.setTitle(tr("Wi&ndow"));
    mMenuWindowToolbars.setTitle(tr("&Toolbars"));
    mMenuWindowToolbars.addAction(mToolbarFile.toggleViewAction());
    mMenuWindowToolbars.addAction(mToolbarEdit.toggleViewAction());
    mMenuWindowToolbars.addAction(mToolbarTracker.toggleViewAction());
    //mMenuWindowToolbars.addAction(mToolbarSong.toggleViewAction());

    //setupWindowMenu(mMenuWindow);

    mMenuHelp.setTitle(tr("&Help"));
    mMenuHelp.addAction(&mActionAudioDiag);
    mMenuHelp.addSeparator();
    mMenuHelp.addAction(&mActionHelpAbout);
    mMenuHelp.addAction(&mActionHelpAboutQt);

    // MENUBAR ================================================================

    auto menubar = menuBar();
    menubar->addMenu(&mMenuFile);
    menubar->addMenu(&mMenuEdit);
    menubar->addMenu(&mMenuTracker);
    menubar->addMenu(&mMenuWindow);
    menubar->addMenu(&mMenuHelp);

    // TOOLBARS ==============================================================

    QSize const iconSize(16, 16);

    mToolbarFile.setWindowTitle(tr("File"));
    mToolbarFile.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarFile);
    mToolbarFile.addAction(&mActionFileNew);
    mToolbarFile.addAction(&mActionFileOpen);
    mToolbarFile.addAction(&mActionFileSave);
    mToolbarFile.addSeparator();
    mToolbarFile.addAction(&mActionFileConfig);

    mToolbarEdit.setWindowTitle(tr("Edit"));
    mToolbarEdit.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarEdit);
    //mToolbarEdit.addAction(mActionEditUndo);
    //mToolbarEdit.addAction(mActionEditRedo);
    mToolbarEdit.addSeparator();
    /*mToolbarEdit.addAction(&patternActions.cut);
    mToolbarEdit.addAction(&patternActions.copy);
    mToolbarEdit.addAction(&patternActions.paste);*/

    mToolbarTracker.setWindowTitle(tr("Tracker"));
    mToolbarTracker.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarTracker);
    mToolbarTracker.addAction(&mActionTrackerPlay);
    mToolbarTracker.addAction(&mActionTrackerRestart);
    mToolbarTracker.addAction(&mActionTrackerStop);

    // DOCKS =================================================================

    

    // STATUSBAR ==============================================================

    auto statusbar = statusBar();
    
    mStatusRenderer.setText(tr("Ready"));
    statusbar->addPermanentWidget(&mStatusRenderer);
    statusbar->addPermanentWidget(&mStatusFramerate);
    statusbar->addPermanentWidget(&mStatusSpeed);
    statusbar->addPermanentWidget(&mStatusTempo);
    mStatusElapsed.setText(QStringLiteral("00:00:00"));
    statusbar->addPermanentWidget(&mStatusElapsed);
    mStatusPos.setText(QStringLiteral("00 / 00"));
    statusbar->addPermanentWidget(&mStatusPos);
    statusbar->addPermanentWidget(&mStatusSamplerate);


    // CONNECTIONS ============================================================

    // Actions

    // File
    connectActionToThis(mActionFileNew, onFileNew);
    connectActionToThis(mActionFileOpen, onFileOpen);
    connectActionToThis(mActionFileSave, onFileSave);
    connectActionToThis(mActionFileSaveAs, onFileSaveAs);
    connect(&mActionFileClose, &QAction::triggered, &mMdi, &QMdiArea::closeActiveSubWindow);
    connect(&mActionFileCloseAll, &QAction::triggered, &mMdi, &QMdiArea::closeAllSubWindows);
    connectActionToThis(mActionFileQuit, close);
    connectActionToThis(mActionWindowResetLayout, onWindowResetLayout);
    connect(&mActionWindowPrev, &QAction::triggered, &mMdi, &QMdiArea::activatePreviousSubWindow);
    connect(&mActionWindowNext, &QAction::triggered, &mMdi, &QMdiArea::activateNextSubWindow);


    connectActionToThis(mActionFileConfig, showConfigDialog);
    connect(&mActionTrackerPlay, &QAction::triggered, &mApp.renderer, &Renderer::play);
    //connect(&mActionTrackerPlayPattern, &QAction::triggered, &mApp.renderer, &Renderer::playPattern);
    //connect(&mActionTrackerPlayStart, &QAction::triggered, &mApp.renderer, &Renderer::playFromStart);
    //connect(&mActionTrackerPlayCursor, &QAction::triggered, &mApp.renderer, &Renderer::playFromCursor);
    connect(&mActionTrackerStop, &QAction::triggered, &mApp.renderer, &Renderer::stopMusic);

    connectActionToThis(mActionAudioDiag, showAudioDiag);
    QApplication::connect(&mActionHelpAboutQt, &QAction::triggered, &QApplication::aboutQt);


    // sync worker
    connect(&mSyncWorker, &SyncWorker::peaksChanged, &mPeakMeter, &PeakMeter::setPeaks);
    connect(&mSyncWorker, &SyncWorker::positionChanged, this, &MainWindow::trackerPositionChanged);
    connect(&mSyncWorker, &SyncWorker::speedChanged, &mStatusSpeed, &QLabel::setText);

    connect(&mApp.renderer, &Renderer::audioStarted, this, &MainWindow::onAudioStart);
    connect(&mApp.renderer, &Renderer::audioStopped, this, &MainWindow::onAudioStop);
    connect(&mApp.renderer, &Renderer::audioError, this, &MainWindow::onAudioError);

    connect(&mMdi, &QMdiArea::subWindowActivated, this, &MainWindow::onSubWindowActivated);

    connect(&mMenuWindow, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);
}

void MainWindow::initState() {
    // setup default layout
    // +-------------------------------------------------------------------------------+
    // | Toolbars                                                                      |
    // +-------------------------------------------------------------------------------+
    // |                   |                                   |                       |
    // | Song properties   |        Visualizer                 | Instruments           |
    // |       +           +-----------------------------------+                       |
    // |     Songs         |        Pattern Editor             |                       |
    // |                   |                                   |                       |
    // +-------------------+                                   |                       |
    // |                   |                                   |                       |
    // | Module properties |                                   |                       |
    // |                   |                                   |                       | 
    // +-------------------+                                   |                       |
    // |                   |                                   |                       |
    // | Song Order        |                                   |                       |
    // |                   |                                   |                       |
    // |                   |                                   +-----------------------+
    // |                   |                                   |                       |
    // |                   |                                   | Waveforms             |
    // |                   |                                   |                       |
    // |                   |                                   |                       |
    // |                   |                                   |                       |
    // |                   |                                   |                       |
    // +-------------------------------------------------------------------------------+
    // Default locations for all dock widgets + toolbars
    // The user can move these or hide these however they like
    // This layout can be restored by invoking menu action Window | Reset Layout

    // setup corners, left and right get both corners
    setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);
    setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);


    addToolBar(Qt::TopToolBarArea, &mToolbarFile);
    mToolbarFile.show();

    addToolBar(Qt::TopToolBarArea, &mToolbarEdit);
    mToolbarEdit.show();

    addToolBar(Qt::TopToolBarArea, &mToolbarTracker);
    mToolbarTracker.show();

}

void MainWindow::setupWindowMenu(QMenu &menu) {

    menu.addSeparator();
    
    menu.addMenu(&mMenuWindowToolbars);
    
    menu.addSeparator();
    
    menu.addAction(&mActionWindowResetLayout);
}

void MainWindow::settingsMessageBox(QMessageBox &msgbox) {
    auto settingsBtn = msgbox.addButton(tr("Change settings"), QMessageBox::ActionRole);
    msgbox.addButton(QMessageBox::Close);
    msgbox.setDefaultButton(settingsBtn);
    msgbox.exec();

    if (msgbox.clickedButton() == settingsBtn) {
        showConfigDialog();
    }
}

//void MainWindow::setupAction(QAction &action, const char *text, const char *tooltip, QKeySequence const &seq) {
//    action.setText(tr(text));
//    action.setShortcut(seq);
//    action.setStatusTip(tr(tooltip));
//}
