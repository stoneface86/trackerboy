
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
    mToolbarTracker()

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
    //mSyncWorker.moveToThread(&mSyncWorkerThread);
    //mSyncWorkerThread.setObjectName(QStringLiteral("sync worker thread"));
    //mSyncWorkerThread.start();
}

MainWindow::~MainWindow() {
    //mSyncWorkerThread.quit();
    //mSyncWorkerThread.wait();
}

QMenu* MainWindow::createPopupMenu() {
    // we can't return a reference to mMenuWindow as QMainWindow will delete it
    // a new menu must be created
    auto menu = new QMenu(this);
    setupWindowMenu(*menu);
    return menu;
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    //if (maybeSave()) {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        evt->accept();
    //} else {
    //    evt->ignore();
    //}
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

    auto doc = ModuleWindow::open(this);
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
    return static_cast<ModuleWindow*>(mMdi.currentSubWindow()->widget())->save();
    /*if (mCurrentDocument->hasFile()) {
        return mCurrentDocument->save();
    } else {
        return onFileSaveAs();
    }*/
}

bool MainWindow::onFileSaveAs() {
    return static_cast<ModuleWindow*>(mMdi.currentSubWindow()->widget())->saveAs();
    /*mModuleFileDialog.setFileMode(QFileDialog::FileMode::AnyFile);
    mModuleFileDialog.setAcceptMode(QFileDialog::AcceptSave);
    mModuleFileDialog.setWindowTitle("Save As");
    if (mModuleFileDialog.exec() != QDialog::Accepted) {
        return false;
    }
    QString filename = mModuleFileDialog.selectedFiles().first();
    auto result = mCurrentDocument->save(filename);
    return result;*/
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
        //mSyncWorker.setSamplesPerFrame(samplesPerFrame);
        //mLeftScope.setDuration(samplesPerFrame);
        //mRightScope.setDuration(samplesPerFrame);

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

        //mPatternEditor.setColors(appearance.colors);
        //mApp.orderModel.setRowColor(appearance.colors[+Color::row]);
    }

    if (categories.testFlag(Config::CategoryKeyboard)) {
        mPianoInput = mApp.config.keyboard().pianoInput;
    }

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
    }

    mConfigDialog->show();
}


void MainWindow::statusSetInstrument(int index) {
    //int id = (index == -1) ? 0 : mApp.instrumentModel.instrument(index)->id();
    //mStatusInstrument.setText(QString("Instrument: %1").arg(id, 2, 16, QChar('0')));
}

void MainWindow::statusSetWaveform(int index) {
    //int id = (index == -1) ? 0 : mApp.waveModel.waveform(index)->id();
    //mStatusWaveform.setText(QString("Waveform: %1").arg(id, 2, 16, QChar('0')));
}

void MainWindow::statusSetOctave(int octave) {
    mStatusOctave.setText(QString("Octave: %1").arg(octave));
    mPianoInput.setOctave(octave);
}

void MainWindow::trackerPositionChanged(QPoint const pos) {
    auto pattern = pos.x();
    auto row = pos.y();
    
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

// PRIVATE METHODS -----------------------------------------------------------

void MainWindow::addDocument(ModuleDocument *doc) {
    auto index = mBrowserModel.addDocument(doc);
    mBrowser.expand(index);

    auto docWin = new ModuleWindow(doc);
    connect(docWin, &ModuleWindow::documentClosed, this, &MainWindow::onDocumentClosed);
    mMdi.addSubWindow(docWin);
    docWin->show();

}

//bool MainWindow::maybeSave() {
//    /*if (!mApp.document.isModified()) {
//        return true;
//    }*/
//
//    auto const result = QMessageBox::warning(
//        this,
//        tr("Trackerboy"),
//        tr("Save changes to \"%1\"?").arg(mDocumentName),
//        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
//        );
//
//    switch (result) {
//        case QMessageBox::Save:
//            return onFileSave();
//        case QMessageBox::Cancel:
//            return false;
//        default:
//            break;
//    }
//
//    return true;
//}

void MainWindow::setModelsEnabled(bool enabled) {
    //mApp.instrumentModel.setEnabled(enabled);
    //mApp.songModel.setEnabled(enabled);
    //mApp.waveModel.setEnabled(enabled);
}

void MainWindow::setupUi() {

    //auto &undoStack = mApp.document.undoStack();

    // CENTRAL WIDGET ========================================================

    // MainWindow expects this to heap-alloc'd as it will manually delete the widget
    mHSplitter = new QSplitter(Qt::Horizontal, this);

    mMdi.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMdi.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mBrowser.setModel(&mBrowserModel);
    mBrowser.setHeaderHidden(true);

    mHSplitter->addWidget(&mBrowser);
    mHSplitter->addWidget(&mMdi);

    setCentralWidget(mHSplitter);

    //mMainWidget = new QWidget(this);

    //mVisLayout.addWidget(&mLeftScope);
    //mVisLayout.addWidget(&mPeakMeter);
    //mVisLayout.addWidget(&mRightScope);


    //mLayout.addLayout(&mVisLayout);
    //mLayout.addWidget(&mPatternEditor);
    //mMainWidget->setLayout(&mLayout);

    //setCentralWidget(mMainWidget);

    //auto &patternActions = mPatternEditor.menuActions();

    // ACTIONS ===============================================================

    setupAction(mActionFileNew, "&New", "Create a new module", Icons::fileNew, QKeySequence::New);
    setupAction(mActionFileOpen, "&Open", "Open an existing module", Icons::fileOpen, QKeySequence::Open);
    setupAction(mActionFileSave, "&Save", "Save the module", Icons::fileSave, QKeySequence::Save);
    setupAction(mActionFileSaveAs, "Save &As...", "Save the module to a new file", QKeySequence::SaveAs);
    setupAction(mActionFileClose, "Close", "Close the current module", QKeySequence::Close);
    setupAction(mActionFileConfig, "&Configuration...", "Change application settings", Icons::fileConfig);
    setupAction(mActionFileQuit, "&Quit", "Exit the application", QKeySequence::Quit);

    /*mActionEditUndo = undoStack.createUndoAction(this);
    mActionEditUndo->setIcon(IconManager::getIcon(Icons::editUndo));
    mActionEditUndo->setShortcut(QKeySequence::Undo);
    mActionEditRedo = undoStack.createRedoAction(this);
    mActionEditRedo->setIcon(IconManager::getIcon(Icons::editRedo));
    mActionEditRedo->setShortcut(QKeySequence::Redo);*/

    //setupAction(mActionSongPrev, "&Previous song", "Selects the previous song in the list", Icons::previous);
    //setupAction(mActionSongNext, "&Next song", "Selects the next song in the list", Icons::next);

    setupAction(mActionTrackerPlay, "&Play", "Resume playing or play the song from the current position", Icons::trackerPlay);
    setupAction(mActionTrackerRestart, "Play from start", "Begin playback of the song from the start", Icons::trackerRestart);
    setupAction(mActionTrackerStop, "&Stop", "Stop playing", Icons::trackerStop);
    setupAction(mActionTrackerToggleChannel, "Toggle channel output", "Enables/disables sound output for the current track");
    setupAction(mActionTrackerSolo, "Solo", "Solos the current track");

    setupAction(mActionWindowResetLayout, "Reset layout", "Rearranges all docks and toolbars to the default layout");

    setupAction(mActionAudioDiag, "Audio diagnostics...", "Shows the audio diagnostics dialog");
    setupAction(mActionHelpAbout, "&About", "About this program");
    setupAction(mActionHelpAboutQt, "About &Qt", "Shows information about Qt");

    // MENUS ==============================================================


    mMenuFile.setTitle(tr("&File"));
    mMenuFile.addAction(&mActionFileNew);
    mMenuFile.addAction(&mActionFileOpen);
    mMenuFile.addAction(&mActionFileSave);
    mMenuFile.addAction(&mActionFileSaveAs);
    mMenuFile.addAction(&mActionFileClose);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileConfig);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileQuit);

    mMenuEdit.setTitle(tr("&Edit"));
    //mMenuEdit.addAction(mActionEditUndo);
    //mMenuEdit.addAction(mActionEditRedo);
    //mMenuEdit.addSeparator();
    //mPatternEditor.setupMenu(mMenuEdit);

    mMenuOrder.setTitle(tr("&Order"));
    //mOrderWidget.setupMenu(mMenuOrder);

    mMenuInstrument.setTitle(tr("&Instrument"));
    //mInstrumentWidget.setupMenu(mMenuInstrument);

    mMenuWaveform.setTitle(tr("&Waveform"));
    //mWaveformWidget.setupMenu(mMenuWaveform);

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

    setupWindowMenu(mMenuWindow);

    mMenuHelp.setTitle(tr("&Help"));
    mMenuHelp.addAction(&mActionAudioDiag);
    mMenuHelp.addSeparator();
    mMenuHelp.addAction(&mActionHelpAbout);
    mMenuHelp.addAction(&mActionHelpAboutQt);

    // MENUBAR ================================================================

    auto menubar = menuBar();
    menubar->addMenu(&mMenuFile);
    menubar->addMenu(&mMenuEdit);
    //menubar->addMenu(&mMenuSong);
    menubar->addMenu(&mMenuOrder);
    menubar->addMenu(&mMenuInstrument);
    menubar->addMenu(&mMenuWaveform);
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


    //mToolbarSong.setWindowTitle(tr("Song"));
    //mToolbarSong.setIconSize(iconSize);
    //setObjectNameFromDeclared(mToolbarSong);
    //mToolbarSong.addAction(&mActionSongPrev);
    //mToolbarSong.addAction(&mActionSongNext);
    //mSongCombo.setModel(&mApp.songModel);
    //mToolbarSong.addWidget(&mSongCombo);

    // DIALOGS ===============================================================

    // DOCKS =================================================================

    

    // STATUSBAR ==============================================================

    auto statusbar = statusBar();
    #define addLabelToStatusbar(var, text) var.setText(QStringLiteral(text)); statusbar->addPermanentWidget(&var)
    
    addLabelToStatusbar(mStatusRenderer, "Ready");
    addLabelToStatusbar(mStatusInstrument, "Instrument: 00");
    addLabelToStatusbar(mStatusWaveform, "Waveform: 00");
    addLabelToStatusbar(mStatusOctave, "Octave: 3");
    addLabelToStatusbar(mStatusFramerate, "59.7 FPS");
    addLabelToStatusbar(mStatusSpeed, "6.000 FPR");
    addLabelToStatusbar(mStatusTempo, "150 BPM");
    addLabelToStatusbar(mStatusElapsed, "00:00:00");
    addLabelToStatusbar(mStatusPos, "00 / 00");
    addLabelToStatusbar(mStatusSamplerate, "");


    // CONNECTIONS ============================================================

    //connect(&mApp.document, &ModuleDocument::modifiedChanged, this, &QMainWindow::setWindowModified);

    // Actions

    // File
    connectActionToThis(mActionFileNew, onFileNew);
    connectActionToThis(mActionFileOpen, onFileOpen);
    connectActionToThis(mActionFileSave, onFileSave);
    connectActionToThis(mActionFileSaveAs, onFileSaveAs);
    connect(&mActionFileClose, &QAction::triggered, &mMdi, &QMdiArea::closeActiveSubWindow);
    connectActionToThis(mActionFileQuit, close);
    connectActionToThis(mActionWindowResetLayout, onWindowResetLayout);

    connectActionToThis(mActionFileConfig, showConfigDialog);
    connect(&mActionTrackerPlay, &QAction::triggered, &mApp.renderer, &Renderer::play);
    //connect(&mActionTrackerPlayPattern, &QAction::triggered, &mApp.renderer, &Renderer::playPattern);
    //connect(&mActionTrackerPlayStart, &QAction::triggered, &mApp.renderer, &Renderer::playFromStart);
    //connect(&mActionTrackerPlayCursor, &QAction::triggered, &mApp.renderer, &Renderer::playFromCursor);
    connect(&mActionTrackerStop, &QAction::triggered, &mApp.renderer, &Renderer::stopMusic);

    connectActionToThis(mActionAudioDiag, showAudioDiag);
    QApplication::connect(&mActionHelpAboutQt, &QAction::triggered, &QApplication::aboutQt);


    // song combobox in mSongToolbar
    //connect(&mSongCombo, qOverload<int>(&QComboBox::currentIndexChanged), &mApp.songModel, qOverload<int>(&SongListModel::select));
    //connect(&mApp.songModel, &SongListModel::currentIndexChanged, &mSongCombo, &QComboBox::setCurrentIndex);

    // statusbar

    //connect(&mApp.instrumentModel, &InstrumentListModel::currentIndexChanged, this, &MainWindow::statusSetInstrument);
    //connect(&mApp.waveModel, &WaveListModel::currentIndexChanged, this, &MainWindow::statusSetWaveform);

    // showEditor signal to each editor's show slot
    //connect(&mInstrumentWidget, &TableForm::showEditor, this, &MainWindow::showInstrumentEditor);
    //connect(&mWaveformWidget, &TableForm::showEditor, this, &MainWindow::showWaveEditor);

    // octave changes
    //connect(&mPatternEditor, &PatternEditor::octaveChanged, this, &MainWindow::statusSetOctave);

    // sync worker
    //connect(&mSyncWorker, &SyncWorker::peaksChanged, &mPeakMeter, &PeakMeter::setPeaks);
    //connect(&mSyncWorker, &SyncWorker::positionChanged, this, &MainWindow::trackerPositionChanged);
    //connect(&mSyncWorker, &SyncWorker::speedChanged, &mStatusSpeed, &QLabel::setText);

    connect(&mApp.renderer, &Renderer::audioStarted, this, &MainWindow::onAudioStart);
    connect(&mApp.renderer, &Renderer::audioStopped, this, &MainWindow::onAudioStop);
    connect(&mApp.renderer, &Renderer::audioError, this, &MainWindow::onAudioError);

    connect(&mMdi, &QMdiArea::subWindowActivated, this, &MainWindow::onSubWindowActivated);
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
