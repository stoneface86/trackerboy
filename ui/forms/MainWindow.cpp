
#include "MainWindow.hpp"

#include "core/samplerates.hpp"
#include "forms/ExportWavDialog.hpp"
#include "misc/IconManager.hpp"
#include "misc/utils.hpp"

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
#include <type_traits>

static const char* MODULE_FILE_FILTER = QT_TR_NOOP("Trackerboy module (*.tbm)");

#define setObjectNameFromDeclared(var) var.setObjectName(QStringLiteral(#var))
#define connectActionToThis(action, slot) connect(&action, &QAction::triggered, this, &MainWindow::slot)
#define connectThis(obj, slot, thisslot) connect(obj, slot, this, &std::remove_pointer_t<decltype(this)>::thisslot)

constexpr int TOOLBAR_ICON_WIDTH = 16;
constexpr int TOOLBAR_ICON_HEIGHT = 16;

MainWindow::MainWindow(Miniaudio &miniaudio) :
    QMainWindow(),
    mMiniaudio(miniaudio),
    mConfig(miniaudio),
    mDocumentCounter(0),
    mErrorSinceLastConfig(false),
    mAboutDialog(nullptr),
    mAudioDiag(nullptr),
    mConfigDialog(nullptr),
    mToolbarFile(),
    mToolbarEdit(),
    mToolbarTracker(),
    mInstrumentEditor(mPianoInput),
    mWaveEditor(mPianoInput),
    mPatternEditor(mPianoInput),
    mRenderer(new Renderer),
    mRenderThread(),
    mSyncWorker(new SyncWorker(*mRenderer)), //, mLeftScope, mRightScope)),
    mSyncWorkerThread()
{
    setupUi();

    // read in application configuration
    mConfig.readSettings();
    

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
        addDockWidget(Qt::LeftDockWidgetArea, &mDockModuleSettings);
        addDockWidget(Qt::LeftDockWidgetArea, &mDockInstrumentEditor);
        addDockWidget(Qt::LeftDockWidgetArea, &mDockWaveformEditor);
        addDockWidget(Qt::LeftDockWidgetArea, &mDockHistory);
        restoreState(windowState);
    }

    // window title
    setWindowTitle(tr("Trackerboy"));

    // audio sync worker thread
    mSyncWorker->moveToThread(&mSyncWorkerThread);
    connect(&mSyncWorkerThread, &QThread::finished, mSyncWorker, &SyncWorker::deleteLater);
    mSyncWorkerThread.setObjectName(QStringLiteral("sync worker thread"));
    mSyncWorkerThread.start();

    // render thread
    mRenderer->moveToThread(&mRenderThread);
    connect(&mRenderThread, &QThread::finished, mRenderer, &Renderer::deleteLater);
    mRenderThread.setObjectName(QStringLiteral("renderer thread"));
    mRenderThread.start();

    // apply the read in configuration
    onConfigApplied(Config::CategoryAll);
}

MainWindow::~MainWindow() {

    // force stop any ongoing render
    //QMetaObject::invokeMethod(mRenderer, &Renderer::forceStop, Qt::BlockingQueuedConnection);
    //mRenderer.forceStop();

    // quit and wait for threads to finish
    mSyncWorkerThread.quit();
   // mUpdateTimerThread.quit();
    mRenderThread.quit();

    mSyncWorkerThread.wait();
    //mUpdateTimerThread.wait();
    mRenderThread.wait();

}

QMenu* MainWindow::createPopupMenu() {
    // we can't return a reference to mMenuView as QMainWindow will delete it
    // a new menu must be created
    auto menu = new QMenu(this);
    setupViewMenu(*menu);
    return menu;
}

void MainWindow::closeEvent(QCloseEvent *evt) {

    onFileCloseAll();

    if (mTabs.currentIndex() == -1) {
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
        tr(MODULE_FILE_FILTER)
    );

    if (path.isEmpty()) {
        return;
    }

    // make sure the document isn't already open
    QFileInfo pathInfo(path);
    int docIndex = 0;
    for (auto doc : mBrowserModel.documents()) {
        if (doc->hasFile()) {
            QFileInfo docInfo(doc->filepath());
            if (pathInfo == docInfo) {
                mTabs.setCurrentIndex(docIndex);
                return; // document is already open, don't open it again
            }
        }

        ++docIndex;
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

void MainWindow::onFileSave() {
    saveDocument(mBrowserModel.currentDocument());
}

void MainWindow::onFileSaveAs() {
    saveDocumentAs(mBrowserModel.currentDocument());
}

void MainWindow::onFileClose() {
    closeDocument(mBrowserModel.currentDocument());
}

void MainWindow::onFileCloseAll() {
    auto &docs = mBrowserModel.documents();
    auto numberToClose = docs.size();
    int indexToClose = 0;

    for (int i = 0; i < numberToClose; ++i) {
        if (!closeDocument(docs[indexToClose])) {
            // user did not close this document, skip it
            indexToClose++;
        }
    }
}

void MainWindow::onViewResetLayout() {
    // remove everything
    removeToolBar(&mToolbarFile);
    removeToolBar(&mToolbarEdit);
    removeToolBar(&mToolbarTracker);

    initState();
}

void MainWindow::onWindowPrevious() {
    int index = mTabs.currentIndex() - 1;
    if (index < 0) {
        index = mTabs.count() - 1;
    }
    mTabs.setCurrentIndex(index);
}

void MainWindow::onWindowNext() {
    int index = mTabs.currentIndex() + 1;
    if (index >= mTabs.count()) {
        index = 0;
    }
    mTabs.setCurrentIndex(index);
}

void MainWindow::onConfigApplied(Config::Categories categories) {
    if (categories.testFlag(Config::CategorySound)) {
        auto &sound = mConfig.sound();
        auto samplerate = SAMPLERATE_TABLE[sound.samplerateIndex];
        mStatusSamplerate.setText(tr("%1 Hz").arg(samplerate));

        auto samplesPerFrame = samplerate / 60;
        mSyncWorker->setSamplesPerFrame(samplesPerFrame);
        //mLeftScope.setDuration(samplesPerFrame);
        //mRightScope.setDuration(samplesPerFrame);

        mRenderer->setConfig(mMiniaudio, sound);
        mErrorSinceLastConfig = mRenderer->lastDeviceError() != MA_SUCCESS;
        if (isVisible() && mErrorSinceLastConfig) {
            QMessageBox msgbox(this);
            msgbox.setIcon(QMessageBox::Critical);
            msgbox.setText(tr("Could not initialize device"));
            msgbox.setInformativeText(tr("The configured device could not be initialized. Playback is disabled."));
            settingsMessageBox(msgbox);
        }
    }

    if (categories.testFlag(Config::CategoryAppearance)) {
        auto &appearance = mConfig.appearance();

        // see resources/stylesheet.qss
        QFile styleFile(QStringLiteral(":/stylesheet.qss"));
        styleFile.open(QFile::ReadOnly);

        QString stylesheet(styleFile.readAll());

        setStyleSheet(stylesheet.arg(
            appearance.colors[+Color::background].name(),
            appearance.colors[+Color::line].name(),
            appearance.colors[+Color::foreground].name(),
            appearance.colors[+Color::selection].name(),
            appearance.font.family(),
            QString::number(appearance.font.pointSize())
        ));

        mPatternEditor.setColors(appearance.colors);
        OrderModel::setRowColor(appearance.colors[+Color::row]);
    }

    if (categories.testFlag(Config::CategoryKeyboard)) {
        mPianoInput = mConfig.keyboard().pianoInput;
    }

    mConfig.writeSettings();
}

void MainWindow::showAboutDialog() {
    if (mAboutDialog == nullptr) {
        mAboutDialog = new AboutDialog(this);
    }
    mAboutDialog->show();
}

void MainWindow::showAudioDiag() {
    if (mAudioDiag == nullptr) {
        mAudioDiag = new AudioDiagDialog(*mRenderer, this);
    }

    mAudioDiag->show();
}

void MainWindow::showConfigDialog() {
    if (mConfigDialog == nullptr) {
        mConfigDialog = new ConfigDialog(mConfig, this);
        mConfigDialog->resetControls();

        connect(mConfigDialog, &ConfigDialog::applied, this, &MainWindow::onConfigApplied);
    }

    mConfigDialog->show();
}

void MainWindow::showExportWavDialog() {
    auto dialog = new ExportWavDialog(*mBrowserModel.currentDocument(), mConfig, this);
    dialog->show();
    dialog->exec();
    delete dialog;
}

void MainWindow::trackerPositionChanged(QPoint const pos) {
    auto pattern = pos.x();
    auto row = pos.y();
    
    auto doc = mBrowserModel.currentDocument();
    if (doc == mRenderer->documentPlayingMusic()) {
        doc->patternModel().setTrackerCursor(row, pattern);
    }

    mStatusPos.setText(QStringLiteral("%1 / %2").arg(pattern).arg(row));
}

void MainWindow::onAudioStart() {
    mStatusRenderer.setText(tr("Playing"));
    auto doc = mRenderer->documentPlayingMusic();
    if (doc) {
        doc->patternModel().setPlaying(true);
    }
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
    onAudioStop();
}

void MainWindow::onAudioStop() {
    if (!mErrorSinceLastConfig) {
        mStatusRenderer.setText(tr("Ready"));
    }

    auto doc = mRenderer->documentPlayingMusic();
    if (doc) {
        doc->patternModel().setPlaying(false);
    }
}

void MainWindow::onTabChanged(int tabIndex) {
    mBrowserModel.setCurrentDocument(tabIndex);
    bool const hasDocument = tabIndex != -1;

    mActionFileSave.setEnabled(hasDocument);
    mActionFileSaveAs.setEnabled(hasDocument);
    mActionFileClose.setEnabled(hasDocument);
    mActionFileCloseAll.setEnabled(hasDocument);
    mActionFileExportWav.setEnabled(hasDocument);
    mActionWindowNext.setEnabled(hasDocument);
    mActionWindowPrev.setEnabled(hasDocument);

    mOrderEditor.setVisible(hasDocument);
    mPatternEditor.setVisible(hasDocument);
    mTabs.setVisible(hasDocument);

    if (hasDocument) {
        mPatternEditor.setFocus();
    }

    updateWindowTitle();

}

void MainWindow::onBrowserDoubleClick(QModelIndex const& index) {

    auto tabIndex = mBrowserModel.documentIndex(index);
    mTabs.setCurrentIndex(tabIndex);

    QDockWidget *dockToActivate = nullptr;

    // "open" the item according to its type
    switch (mBrowserModel.itemAt(index)) {
        case ModuleModel::ItemType::invalid:
        case ModuleModel::ItemType::document:
        case ModuleModel::ItemType::orders:
            break;
        case ModuleModel::ItemType::instrument:
            mInstrumentEditor.openItem(index.row());
            [[fallthrough]];
        case ModuleModel::ItemType::instruments:
            dockToActivate = &mDockInstrumentEditor;
            break;
        case ModuleModel::ItemType::order:
            mBrowserModel.documents()[tabIndex]->orderModel().selectPattern(index.row());
            break;
        case ModuleModel::ItemType::waveform:
            mWaveEditor.openItem(index.row());
            [[fallthrough]];
        case ModuleModel::ItemType::waveforms:
            dockToActivate = &mDockWaveformEditor;
            break;
        case ModuleModel::ItemType::settings:
            dockToActivate = &mDockModuleSettings;
            break;
    }

    if (dockToActivate) {
        dockToActivate->show();
        dockToActivate->raise();
        dockToActivate->activateWindow();
    }
}

void MainWindow::onDocumentModified(bool modified) {
    auto doc = qobject_cast<ModuleDocument*>(sender());
    int index = mBrowserModel.documents().indexOf(doc);

    // this happens when the document is being destroyed for some reason
    if (index == -1) {
        return;
    }

    if (modified) {
        mTabs.setTabText(index, QStringLiteral("%1*").arg(doc->name()));
    } else {
        mTabs.setTabText(index, doc->name());
    }

    if (index == mTabs.currentIndex()) {
        updateWindowTitle();
    }
}

void MainWindow::updateWindowMenu() {
    mMenuWindow.clear();
    mMenuWindow.addAction(&mActionWindowPrev);
    mMenuWindow.addAction(&mActionWindowNext);
    
    auto const& documents = mBrowserModel.documents();
    if (documents.size() > 0) {
        mMenuWindow.addSeparator();

        int i = 1;
        int const current = mTabs.currentIndex() + 1;
        for (auto doc : documents) {
            
            auto textFmt = (i < 9) ? "&%1 %2" : "%1 %2";
            QString text = tr(textFmt).arg(i).arg(doc->name());

            auto action = mMenuWindow.addAction(text, this, [this, i]() {
                mTabs.setCurrentIndex(i - 1);
                });
            action->setCheckable(true);
            action->setChecked(i == current);

            ++i;
        }
    }

}

void MainWindow::closeTab(int index) {
    closeDocument(mBrowserModel.documents()[index]);
}

// PRIVATE METHODS -----------------------------------------------------------


void MainWindow::addDocument(ModuleDocument *doc) {
    // add the document to the model
    auto index = mBrowserModel.addDocument(doc);
    // expand the index of the newly added model
    //mBrowser.expandRecursively(index); // cannot use, added in 5.13
    expandRecursively(mBrowser, index); // use this workaround instead

    connect(doc, &ModuleDocument::modifiedChanged, this, &MainWindow::onDocumentModified);

    auto tabindex = mTabs.addTab(doc->name());
    mTabs.setCurrentIndex(tabindex);

    doc->undoStack().setUndoLimit((int)mConfig.general().historyLimit);
}

bool MainWindow::saveDocument(ModuleDocument *doc) {
    if (doc->hasFile()) {
        // save the document to its file
        return doc->save();
    } else {
        // no associated file, do save as instead
        return saveDocumentAs(doc);
    }
}

bool MainWindow::saveDocumentAs(ModuleDocument *doc) {
    auto path = QFileDialog::getSaveFileName(
        this,
        tr("Save module"),
        "",
        tr(MODULE_FILE_FILTER)
        );

    if (path.isEmpty()) {
        return false;
    }

    auto result = doc->save(path);
    if (result) {
        // the document has a new name, update the tab text and window title
        int index = mBrowserModel.documents().indexOf(doc);
        if (index != -1) {
            // update tab text
            mTabs.setTabText(index, doc->name());
            if (index == mTabs.currentIndex()) {
                // update window title if this tab is the current one
                // (should always be but we'll check anyways)
                updateWindowTitle();
            }
        }
    }
    return result;
}

bool MainWindow::closeDocument(ModuleDocument *doc) {
    if (doc->isModified()) {
        // prompt the user if they want to save any changes
        auto const result = QMessageBox::warning(
            this,
            tr("Trackerboy"),
            tr("Save changes to %1?").arg(doc->name()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        switch (result) {
            case QMessageBox::Save:
                if (!saveDocument(doc)) {
                    // save failed, do not close document
                    return false;
                }
                break;
            case QMessageBox::Cancel:
                // user cancelled, do not close document
                return false;
            default:
                break;
        }
    }

    auto tabIndex = mBrowserModel.documents().indexOf(doc);
    mBrowserModel.removeDocument(doc);
    mTabs.removeTab(tabIndex);

    // TODO: renderer needs to know about the document being removed
    mRenderer->removeDocument(doc);
    // if (mRenderer.document() == doc) {
    // }

    // no longer using this document, delete it
    delete doc;

    // the document was closed, return true
    return true;
}

void MainWindow::setupUi() {

    // CENTRAL WIDGET ========================================================

    // MainWindow expects this to heap-alloc'd as it will manually delete the widget
    mHSplitter = new QSplitter(Qt::Horizontal, this);

    mTabs.setDocumentMode(true);
    mTabs.setTabsClosable(true);
    mTabs.setMovable(true);

    mBrowser.setModel(&mBrowserModel);
    mBrowser.setHeaderHidden(true);
    mBrowser.setExpandsOnDoubleClick(false);

    //mVisualizerLayout.addWidget(&mLeftScope);
    //mVisualizerLayout.addWidget(&mPeakMeter, 1);
    //mVisualizerLayout.addWidget(&mRightScope);

    mEditorLayout.addWidget(&mOrderEditor);
    mEditorLayout.addWidget(&mPatternEditor, 1);
    mEditorLayout.setMargin(0);

    //mMainLayout.addLayout(&mVisualizerLayout);
    mMainLayout.addWidget(&mTabs);
    mMainLayout.addLayout(&mEditorLayout, 1);
    mMainLayout.setMargin(0);
    mMainWidget.setLayout(&mMainLayout);

    mHSplitter->addWidget(&mBrowser);
    mHSplitter->addWidget(&mMainWidget);
    mHSplitter->setStretchFactor(0, 0);
    mHSplitter->setStretchFactor(1, 1);
    setCentralWidget(mHSplitter);

    mTabs.setVisible(false);
    mOrderEditor.setVisible(false);
    mPatternEditor.setVisible(false);

    // ACTIONS ===============================================================

    setupAction(mActionFileNew, "&New", "Create a new module", Icons::fileNew, QKeySequence::New);
    setupAction(mActionFileOpen, "&Open", "Open an existing module", Icons::fileOpen, QKeySequence::Open);
    setupAction(mActionFileSave, "&Save", "Save the module", Icons::fileSave, QKeySequence::Save);
    setupAction(mActionFileSaveAs, "Save &As...", "Save the module to a new file", QKeySequence::SaveAs);
    setupAction(mActionFileExportWav, "Export to WAV...", "Exports the module to a WAV file");
    setupAction(mActionFileClose, "Close", "Close the current module", QKeySequence::Close);
    setupAction(mActionFileCloseAll, "Close All", "Closes all open modules");
    setupAction(mActionFileConfig, "&Configuration...", "Change application settings", Icons::fileConfig);
    setupAction(mActionFileQuit, "&Quit", "Exit the application", QKeySequence::Quit);

    auto &undoGroup = mBrowserModel.undoGroup();
    mActionEditUndo = undoGroup.createUndoAction(this);
    mActionEditUndo->setIcon(IconManager::getIcon(Icons::editUndo));
    mActionEditUndo->setShortcut(QKeySequence::Undo);
    mActionEditRedo = undoGroup.createRedoAction(this);
    mActionEditRedo->setIcon(IconManager::getIcon(Icons::editRedo));
    mActionEditRedo->setShortcut(QKeySequence::Redo);

    setupAction(mActionViewResetLayout, "Reset layout", "Rearranges all docks and toolbars to the default layout");

    setupAction(mActionTrackerPlay, "&Play", "Resume playing or play the song from the current position", Icons::trackerPlay);
    setupAction(mActionTrackerRestart, "Play from start", "Begin playback of the song from the start", Icons::trackerRestart);
    setupAction(mActionTrackerStop, "&Stop", "Stop playing", Icons::trackerStop);
    setupAction(mActionTrackerToggleChannel, "Toggle channel output", "Enables/disables sound output for the current track");
    setupAction(mActionTrackerSolo, "Solo", "Solos the current track");
    setupAction(mActionTrackerKill, "&Kill sound", "Immediately stops sound output", QKeySequence(Qt::Key_F12));

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
    mActionFileExportWav.setEnabled(false);

    // MENUS ==============================================================


    mMenuFile.setTitle(tr("&File"));
    mMenuFile.addAction(&mActionFileNew);
    mMenuFile.addAction(&mActionFileOpen);
    mMenuFile.addAction(&mActionFileSave);
    mMenuFile.addAction(&mActionFileSaveAs);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileExportWav);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileClose);
    mMenuFile.addAction(&mActionFileCloseAll);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileConfig);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileQuit);

    mMenuEdit.setTitle(tr("&Edit"));
    mMenuEdit.addAction(mActionEditUndo);
    mMenuEdit.addAction(mActionEditRedo);
    mMenuEdit.addSeparator();
    mPatternEditor.setupMenu(mMenuEdit);
    // mMenuEdit.addAction(&patternActions.cut]);
    // mMenuEdit.addAction(&patternActions.copy);
    // mMenuEdit.addAction(&patternActions.paste);
    // mMenuEdit.addAction(&patternActions.pasteMix);
    // mMenuEdit.addAction(&patternActions.delete_);
    // mMenuEdit.addSeparator();
    // mMenuEdit.addAction(&mActionEditSelectAll);
    // mMenuEdit.addSeparator();
    //     mMenuEditTranspose.setTitle(tr("Transpose"));
    //     mMenuEditTranspose.addAction(&mActionTransposeNoteIncrease);
    //     mMenuEditTranspose.addAction(&mActionTransposeNoteDecrease);
    //     mMenuEditTranspose.addAction(&mActionTransposeOctaveIncrease);
    //     mMenuEditTranspose.addAction(&mActionTransposeOctaveDecrease);
    // mMenuEdit.addMenu(&mMenuEditTranspose);    

    mMenuView.setTitle(tr("&View"));
    mMenuViewToolbars.setTitle(tr("&Toolbars"));
    mMenuViewToolbars.addAction(mToolbarFile.toggleViewAction());
    mMenuViewToolbars.addAction(mToolbarEdit.toggleViewAction());
    mMenuViewToolbars.addAction(mToolbarTracker.toggleViewAction());
    setupViewMenu(mMenuView);

    mMenuTracker.setTitle(tr("&Tracker"));
    mMenuTracker.addAction(&mActionTrackerPlay);
    mMenuTracker.addAction(&mActionTrackerRestart);
    mMenuTracker.addAction(&mActionTrackerStop);
    mMenuTracker.addSeparator();

    mMenuTracker.addAction(&mActionTrackerToggleChannel);
    mMenuTracker.addAction(&mActionTrackerSolo);

    mMenuTracker.addSeparator();

    mMenuTracker.addAction(&mActionTrackerKill);

    mMenuWindow.setTitle(tr("Wi&ndow"));

    mMenuHelp.setTitle(tr("&Help"));
    mMenuHelp.addAction(&mActionAudioDiag);
    mMenuHelp.addSeparator();
    mMenuHelp.addAction(&mActionHelpAbout);
    mMenuHelp.addAction(&mActionHelpAboutQt);

    // MENUBAR ================================================================

    auto menubar = menuBar();
    menubar->addMenu(&mMenuFile);
    menubar->addMenu(&mMenuEdit);
    menubar->addMenu(&mMenuView);
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
    mToolbarEdit.addAction(mActionEditUndo);
    mToolbarEdit.addAction(mActionEditRedo);
    mToolbarEdit.addSeparator();
    auto &patternActions = mPatternEditor.menuActions();
    mToolbarEdit.addAction(&patternActions.cut);
    mToolbarEdit.addAction(&patternActions.copy);
    mToolbarEdit.addAction(&patternActions.paste);

    mToolbarTracker.setWindowTitle(tr("Tracker"));
    mToolbarTracker.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarTracker);
    mToolbarTracker.addAction(&mActionTrackerPlay);
    mToolbarTracker.addAction(&mActionTrackerRestart);
    mToolbarTracker.addAction(&mActionTrackerStop);

    // DOCKS =================================================================

    setObjectNameFromDeclared(mDockModuleSettings);
    mDockModuleSettings.setWindowTitle(tr("Module settings"));
    mDockModuleSettings.setWidget(&mModuleSettingsWidget);
    
    setObjectNameFromDeclared(mDockInstrumentEditor);
    mDockInstrumentEditor.setWindowTitle(tr("Instrument editor"));
    mDockInstrumentEditor.setWidget(&mInstrumentEditor);
    
    setObjectNameFromDeclared(mDockWaveformEditor);
    mDockWaveformEditor.setWindowTitle(tr("Waveform editor"));
    mDockWaveformEditor.setWidget(&mWaveEditor);

    setObjectNameFromDeclared(mDockHistory);
    mDockHistory.setWindowTitle(tr("History"));
    mDockHistory.setWidget(&mUndoView);
    mUndoView.setGroup(&undoGroup);

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
    connectActionToThis(mActionFileExportWav, showExportWavDialog);
    connectActionToThis(mActionFileClose, onFileClose);
    connectActionToThis(mActionFileCloseAll, onFileCloseAll);
    connectActionToThis(mActionFileConfig, showConfigDialog);
    connectActionToThis(mActionFileQuit, close);

    // view
    connectActionToThis(mActionViewResetLayout, onViewResetLayout);

    // window
    connectActionToThis(mActionWindowPrev, onWindowPrevious);
    connectActionToThis(mActionWindowNext, onWindowNext);

    // tracker
    connect(&mActionTrackerPlay, &QAction::triggered, mRenderer, &Renderer::play, Qt::QueuedConnection);
    connect(&mActionTrackerRestart, &QAction::triggered, mRenderer, &Renderer::playAtStart, Qt::QueuedConnection);
    connect(&mActionTrackerStop, &QAction::triggered, mRenderer, &Renderer::stopMusic, Qt::QueuedConnection);
    connect(&mActionTrackerKill, &QAction::triggered, mRenderer, &Renderer::forceStop, Qt::QueuedConnection);

    // help
    connectActionToThis(mActionAudioDiag, showAudioDiag);
    connectActionToThis(mActionHelpAbout, showAboutDialog);
    QApplication::connect(&mActionHelpAboutQt, &QAction::triggered, &QApplication::aboutQt);

    // editors
    {
        auto &piano = mInstrumentEditor.piano();
        connect(&piano, &PianoWidget::keyDown, mRenderer, &Renderer::previewInstrument, Qt::QueuedConnection);
        connect(&piano, &PianoWidget::keyUp, mRenderer, &Renderer::stopPreview, Qt::QueuedConnection);
    }

    connect(&mInstrumentEditor, &InstrumentEditor::openWaveEditor, this,
        [this](int index) {
            mWaveEditor.openItem(index);
            mDockWaveformEditor.show();
            mDockWaveformEditor.raise();
            mDockWaveformEditor.activateWindow();
        });

    {
        auto &piano = mWaveEditor.piano();
        connect(&piano, &PianoWidget::keyDown, mRenderer, &Renderer::previewWaveform, Qt::QueuedConnection);
        connect(&piano, &PianoWidget::keyUp, mRenderer, &Renderer::stopPreview, Qt::QueuedConnection);
    }

    connect(&mPatternEditor, &PatternEditor::octaveChanged, this, [this](int octave){ 
        mPianoInput.setOctave(octave); 
    });

    // sync worker
    //connect(mSyncWorker, &SyncWorker::peaksChanged, &mPeakMeter, &PeakMeter::setPeaks, Qt::QueuedConnection);
    connect(mSyncWorker, &SyncWorker::positionChanged, this, &MainWindow::trackerPositionChanged, Qt::QueuedConnection);
    connect(mSyncWorker, &SyncWorker::speedChanged, &mStatusSpeed, &QLabel::setText, Qt::QueuedConnection);

    connect(mRenderer, &Renderer::audioStarted, this, &MainWindow::onAudioStart, Qt::QueuedConnection);
    connect(mRenderer, &Renderer::audioStopped, this, &MainWindow::onAudioStop, Qt::QueuedConnection);
    connect(mRenderer, &Renderer::audioError, this, &MainWindow::onAudioError, Qt::QueuedConnection);

    connect(&mMenuWindow, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);

    connect(&mBrowser, &QTreeView::doubleClicked, this, &MainWindow::onBrowserDoubleClick);
    connect(&mBrowserModel, &ModuleModel::currentDocumentChanged, &mModuleSettingsWidget, &ModuleSettingsWidget::setDocument);
    connect(&mBrowserModel, &ModuleModel::currentDocumentChanged, &mInstrumentEditor, &InstrumentEditor::setDocument);
    connect(&mBrowserModel, &ModuleModel::currentDocumentChanged, &mWaveEditor, &WaveEditor::setDocument);
    connect(&mBrowserModel, &ModuleModel::currentDocumentChanged, mRenderer, &Renderer::setDocument);
    connect(&mBrowserModel, &ModuleModel::currentDocumentChanged, &mOrderEditor, &OrderEditor::setDocument);
    connect(&mBrowserModel, &ModuleModel::currentDocumentChanged, &mPatternEditor, &PatternEditor::setDocument);
    

    //connect(&mRenderer, &Renderer::audioStarted, &mUpdateTimer, qOverload<>(&QTimer::start), Qt::QueuedConnection);
    //connect(&mRenderer, &Renderer::audioStopped, &mUpdateTimer, &QTimer::stop, Qt::QueuedConnection);
    //connect(&mRenderer, &Renderer::audioError, &mUpdateTimer, &QTimer::stop, Qt::QueuedConnection);
    
    connectThis(&mTabs, &QTabBar::currentChanged, onTabChanged);
    connectThis(&mTabs, &QTabBar::tabCloseRequested, closeTab);
    connect(&mTabs, &QTabBar::tabMoved, &mBrowserModel, &ModuleModel::moveDocument);

}

void MainWindow::initState() {
    // setup default layout

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

    addDockWidget(Qt::RightDockWidgetArea, &mDockModuleSettings);
    mDockModuleSettings.setFloating(true);
    mDockModuleSettings.hide();

    addDockWidget(Qt::RightDockWidgetArea, &mDockInstrumentEditor);
    mDockInstrumentEditor.setFloating(true);
    mDockInstrumentEditor.hide();

    addDockWidget(Qt::RightDockWidgetArea, &mDockWaveformEditor);
    mDockWaveformEditor.setFloating(true);
    mDockWaveformEditor.hide();

    addDockWidget(Qt::RightDockWidgetArea, &mDockHistory);
    mDockHistory.setFloating(true);
    mDockHistory.hide();
}

void MainWindow::setupViewMenu(QMenu &menu) {

    menu.addAction(mDockModuleSettings.toggleViewAction());
    menu.addAction(mDockInstrumentEditor.toggleViewAction());
    menu.addAction(mDockWaveformEditor.toggleViewAction());
    menu.addAction(mDockHistory.toggleViewAction());

    menu.addSeparator();
    
    menu.addMenu(&mMenuViewToolbars);
    
    menu.addSeparator();
    
    menu.addAction(&mActionViewResetLayout);
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

void MainWindow::updateWindowTitle() {
    int current = mTabs.currentIndex();
    if (current != -1) {
        setWindowTitle(tr("%1 - Trackerboy").arg(mTabs.tabText(current)));
    } else {
        setWindowTitle(tr("Trackerboy"));
    }
}
