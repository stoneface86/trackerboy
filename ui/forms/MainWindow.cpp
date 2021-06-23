
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
    mPianoInput(),
    mDocumentCounter(0),
    mErrorSinceLastConfig(false),
    mAboutDialog(nullptr),
    mAudioDiag(nullptr),
    mConfigDialog(nullptr),
    mToolbarFile(),
    mToolbarEdit(),
    mToolbarTracker(),
    mToolbarInput(),
    mOctaveLabel(tr("Octave")),
    mOctaveSpin(),
    mEditStepLabel(tr("Edit step")),
    mEditStepSpin(),
    mToolbarInstrument(),
    mInstrumentCombo(),
    mDockModuleSettings(),
    mModuleSettingsWidget(),
    mDockInstrumentEditor(),
    mInstrumentEditor(mPianoInput),
    mDockWaveformEditor(),
    mWaveEditor(mPianoInput),
    mDockHistory(),
    mUndoView(),
    mHSplitter(nullptr),
    mBrowser(),
    mMainWidget(),
    mMainLayout(),
    mTabs(),
    mEditorWidget(),
    mEditorLayout(),
    // mOrderGroup(tr("Song order")),
    // mOrderLayout(),
    // mOrderEditor(),
    // mSongGroup(tr("Song settings")),
    mSidebar(),
    mPatternEditor(mPianoInput),
    mPlayAndStopShortcut(&mPatternEditor),
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
        addToolBar(&mToolbarSong);
        addToolBar(&mToolbarTracker);
        addToolBar(&mToolbarInput);
        addToolBar(&mToolbarInstrument);
        addDockWidget(Qt::LeftDockWidgetArea, &mDockModuleSettings);
        addDockWidget(Qt::LeftDockWidgetArea, &mDockInstrumentEditor);
        addDockWidget(Qt::LeftDockWidgetArea, &mDockWaveformEditor);
        addDockWidget(Qt::LeftDockWidgetArea, &mDockHistory);
        restoreState(windowState);
    }

    // initialize document state
    onTabChanged(-1);

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
    auto previousDocument = mBrowserModel.currentDocument();
    mBrowserModel.setCurrentDocument(tabIndex);
    bool const hasDocument = tabIndex != -1;

    if (mActions[DOCUMENT_ACTIONS_BEGIN].isEnabled() != hasDocument) {
        // update enabled state for all document actions
        for (int i = DOCUMENT_ACTIONS_BEGIN; i < ACTION_COUNT; ++i) {
            mActions[i].setEnabled(hasDocument);
        }
    }

    mPlayAndStopShortcut.setEnabled(hasDocument);

    if (previousDocument) {
        // disconnect any signals for the previous document
        auto &patternModel = previousDocument->patternModel();
        patternModel.disconnect(&mActions[ActionTrackerRecord]);
        mActions[ActionTrackerRecord].disconnect(&patternModel);
        mActions[ActionTrackerFollow].disconnect(&patternModel);
        mActions[ActionEditKeyRepetition].disconnect(previousDocument);
        previousDocument->setInstrument(mInstrumentCombo.currentIndex());
        previousDocument->orderModel().disconnect(this);
    }

    if (hasDocument) {
        auto doc = mBrowserModel.currentDocument();
        auto &patternModel = doc->patternModel();
        mActions[ActionTrackerRecord].setChecked(patternModel.isRecording());
        connect(&mActions[ActionTrackerRecord], &QAction::toggled, &patternModel, &PatternModel::setRecord);
        connect(&patternModel, &PatternModel::recordingChanged, &mActions[ActionTrackerRecord], &QAction::setChecked);
        
        // follow mode
        mActions[ActionTrackerFollow].setChecked(patternModel.isFollowing());
        connect(&mActions[ActionTrackerFollow], &QAction::toggled, &patternModel, &PatternModel::setFollowing);

        // key repetition
        mActions[ActionEditKeyRepetition].setChecked(doc->keyRepetition());
        connect(&mActions[ActionEditKeyRepetition], &QAction::toggled, doc, &ModuleDocument::setKeyRepetition);

        // edit step
        mEditStepSpin.setValue(doc->editStep());

        mInstrumentChoiceModel.setModel(&doc->instrumentModel());
        auto index = doc->instrument();
        if (index != mInstrumentCombo.currentIndex()) {
            mInstrumentCombo.setCurrentIndex(doc->instrument());
        } else {
            // the index didn't change, but the referenced instrument might have
            mPatternEditor.setInstrument(index);
        }

        auto &orderModel = doc->orderModel();
        // when the user changes the current pattern, we have to make sure the
        // move up/down actions are properly enabled
        connect(&orderModel, &OrderModel::currentPatternChanged, this, &MainWindow::updateOrderActions);
        updateOrderActions();
        
    } else {
        mInstrumentChoiceModel.setModel(nullptr);
        mActions[ActionTrackerRecord].setChecked(false);
    }
    

    mTabs.setVisible(hasDocument);
    mEditorWidget.setVisible(hasDocument);
    
    mOctaveSpin.setEnabled(hasDocument);
    mEditStepSpin.setEnabled(hasDocument);
    mInstrumentCombo.setEnabled(hasDocument);

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
    mMenuWindow.addAction(&mActions[ActionWindowPrev]);
    mMenuWindow.addAction(&mActions[ActionWindowNext]);
    
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

    mRenderer->removeDocument(doc);

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

    //mOrderLayout.addWidget(&mOrderEditor, 0, 0);
    //mOrderGroup.setLayout(&mOrderLayout);

    //mGroupLayout.addWidget(&mOrderGroup, 1);
    //mGroupLayout.addWidget(&mSongGroup);

    mMainWidget.setObjectName(QStringLiteral("MainWidget"));

    //mEditorLayout.addLayout(&mGroupLayout);
    mEditorLayout.addWidget(&mSidebar);
    mEditorLayout.addWidget(&mPatternEditor, 1);
    mEditorLayout.setMargin(0);
    mEditorWidget.setLayout(&mEditorLayout);
    mEditorWidget.setBackgroundRole(QPalette::Base);
    mEditorWidget.setAutoFillBackground(true);

    //mMainLayout.addLayout(&mVisualizerLayout);
    mMainLayout.addWidget(&mTabs);
    //mMainLayout.addLayout(&mEditorLayout, 1);
    mMainLayout.addWidget(&mEditorWidget, 1);
    mMainLayout.setMargin(0);
    mMainLayout.setSpacing(0);
    mMainWidget.setLayout(&mMainLayout);

    mHSplitter->addWidget(&mBrowser);
    mHSplitter->addWidget(&mMainWidget);
    mHSplitter->setStretchFactor(0, 0);
    mHSplitter->setStretchFactor(1, 1);
    setCentralWidget(mHSplitter);

    // ACTIONS ===============================================================

    setupAction(mActions[ActionFileNew], "&New", "Create a new module", Icons::fileNew, QKeySequence::New);
    setupAction(mActions[ActionFileOpen], "&Open", "Open an existing module", Icons::fileOpen, QKeySequence::Open);
    setupAction(mActions[ActionFileSave], "&Save", "Save the module", Icons::fileSave, QKeySequence::Save);
    setupAction(mActions[ActionFileSaveAs], "Save &As...", "Save the module to a new file", QKeySequence::SaveAs);
    setupAction(mActions[ActionFileExportWav], "Export to WAV...", "Exports the module to a WAV file");
    setupAction(mActions[ActionFileClose], "Close", "Close the current module", QKeySequence::Close);
    setupAction(mActions[ActionFileCloseAll], "Close All", "Closes all open modules");
    setupAction(mActions[ActionFileConfig], "&Configuration...", "Change application settings", Icons::fileConfig);
    setupAction(mActions[ActionFileQuit], "&Quit", "Exit the application", QKeySequence::Quit);

    auto &undoGroup = mBrowserModel.undoGroup();
    mActionEditUndo = undoGroup.createUndoAction(this);
    mActionEditUndo->setIcon(IconManager::getIcon(Icons::editUndo));
    mActionEditUndo->setShortcut(QKeySequence::Undo);
    mActionEditRedo = undoGroup.createRedoAction(this);
    mActionEditRedo->setIcon(IconManager::getIcon(Icons::editRedo));
    mActionEditRedo->setShortcut(QKeySequence::Redo);

    setupAction(mActions[ActionEditCut], "C&ut", "Copies and deletes selection to the clipboard", Icons::editCut, QKeySequence::Cut);
    setupAction(mActions[ActionEditCopy], "&Copy", "Copies selected rows to the clipboard", Icons::editCopy, QKeySequence::Copy);
    setupAction(mActions[ActionEditPaste], "&Paste", "Pastes contents at the cursor", Icons::editPaste, QKeySequence::Paste);
    setupAction(mActions[ActionEditPasteMix], "Paste &Mix", "Pastes contents at the cursor, merging with existing rows", tr("Ctrl+M"));
    setupAction(mActions[ActionEditErase], "&Erase", "Erases selection contents", QKeySequence::Delete);
    setupAction(mActions[ActionEditSelectAll], "&Select All", "Selects entire track/pattern", QKeySequence::SelectAll);
    setupAction(mActions[ActionEditNoteDecrease], "Decrease note", "Decreases note/notes by 1 step");
    setupAction(mActions[ActionEditNoteIncrease], "Increase note", "Increases note/notes by 1 step");
    setupAction(mActions[ActionEditOctaveDecrease], "Decrease octave", "Decreases note/notes by 12 steps");
    setupAction(mActions[ActionEditOctaveIncrease], "Increase octave", "Increases note/notes by 12 steps");
    setupAction(mActions[ActionEditTranspose], "Custom...", "Transpose by a custom amount", tr("Ctrl+T"));
    setupAction(mActions[ActionEditReverse], "&Reverse", "Reverses selected rows", tr("Ctrl+R"));
    setupAction(mActions[ActionEditKeyRepetition], "Key repetition", "Toggles key repetition for pattern editor");
    mActions[ActionEditKeyRepetition].setCheckable(true);
    mActions[ActionEditKeyRepetition].setChecked(true);

    mMenuTranspose.setTitle(tr("&Transpose"));
    mMenuTranspose.addAction(&mActions[ActionEditNoteDecrease]);
    mMenuTranspose.addAction(&mActions[ActionEditNoteIncrease]);
    mMenuTranspose.addAction(&mActions[ActionEditOctaveDecrease]);
    mMenuTranspose.addAction(&mActions[ActionEditOctaveDecrease]);
    mMenuTranspose.addAction(&mActions[ActionEditTranspose]);

    setupAction(mActions[ActionSongOrderInsert], "&Insert order row", "Inserts a new order at the current pattern", Icons::itemAdd);
    setupAction(mActions[ActionSongOrderRemove], "&Remove order row", "Removes the order at the current pattern", Icons::itemRemove);
    setupAction(mActions[ActionSongOrderDuplicate], "&Duplicate order row", "Duplicates the order at the current pattern", Icons::itemDuplicate);
    setupAction(mActions[ActionSongOrderMoveUp], "Move order &up", "Moves the order up 1", Icons::moveUp);
    setupAction(mActions[ActionSongOrderMoveDown], "Move order dow&n", "Moves the order down 1", Icons::moveDown);

    setupAction(mActions[ActionViewResetLayout], "Reset layout", "Rearranges all docks and toolbars to the default layout");

    setupAction(mActions[ActionTrackerPlay], "&Play", "Resume playing or play the song from the current position", Icons::trackerPlay);
    setupAction(mActions[ActionTrackerRestart], "Play from start", "Begin playback of the song from the start", Icons::trackerRestart, QKeySequence(Qt::Key_F5));
    setupAction(mActions[ActionTrackerPlayCurrentRow], "Play at cursor", "Begin playback from the cursor", Icons::trackerPlayRow, QKeySequence(Qt::Key_F6));
    setupAction(mActions[ActionTrackerStepRow], "Step row", "Play and hold the row at the cursor", Icons::trackerStepRow, QKeySequence(Qt::Key_F7));
    setupAction(mActions[ActionTrackerStop], "&Stop", "Stop playing", Icons::trackerStop, QKeySequence(Qt::Key_F8));
    setupAction(mActions[ActionTrackerRecord], "Record", "Toggles record mode", Icons::trackerRecord, QKeySequence(Qt::Key_Space));
    mActions[ActionTrackerRecord].setCheckable(true);

    setupAction(mActions[ActionTrackerToggleChannel], "Toggle channel output", "Enables/disables sound output for the current track", QKeySequence(Qt::Key_F10));
    setupAction(mActions[ActionTrackerSolo], "Solo", "Solos the current track", QKeySequence(Qt::Key_F11));
    setupAction(mActions[ActionTrackerKill], "&Kill sound", "Immediately stops sound output", QKeySequence(Qt::Key_F12));
    setupAction(mActions[ActionTrackerRepeat], "Pattern repeat", "Toggles pattern repeat mode", Icons::trackerRepeat, QKeySequence(Qt::Key_F9));
    setupAction(mActions[ActionTrackerFollow], "Follow-mode", "Toggles follow mode", Qt::Key_ScrollLock);
    mActions[ActionTrackerRepeat].setCheckable(true);
    mActions[ActionTrackerFollow].setCheckable(true);

    setupAction(mActions[ActionWindowPrev], "Pre&vious", "Move the focus to the previous module");
    setupAction(mActions[ActionWindowNext], "Ne&xt", "Move the focus to the next module");

    setupAction(mActions[ActionHelpAudioDiag], "Audio diagnostics...", "Shows the audio diagnostics dialog");
    setupAction(mActions[ActionHelpAbout], "&About", "About this program");
    setupAction(mActions[ActionHelpAboutQt], "About &Qt", "Shows information about Qt");

    // MENUS ==============================================================


    mMenuFile.setTitle(tr("&File"));
    mMenuFile.addAction(&mActions[ActionFileNew]);
    mMenuFile.addAction(&mActions[ActionFileOpen]);
    mMenuFile.addAction(&mActions[ActionFileSave]);
    mMenuFile.addAction(&mActions[ActionFileSaveAs]);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActions[ActionFileExportWav]);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActions[ActionFileClose]);
    mMenuFile.addAction(&mActions[ActionFileCloseAll]);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActions[ActionFileConfig]);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActions[ActionFileQuit]);

    mMenuEdit.setTitle(tr("&Edit"));
    mMenuEdit.addAction(mActionEditUndo);
    mMenuEdit.addAction(mActionEditRedo);
    mMenuEdit.addSeparator();
    mMenuEdit.addAction(&mActions[ActionEditCut]);
    mMenuEdit.addAction(&mActions[ActionEditCopy]);
    mMenuEdit.addAction(&mActions[ActionEditPaste]);
    mMenuEdit.addAction(&mActions[ActionEditPasteMix]);
    mMenuEdit.addAction(&mActions[ActionEditErase]);
    mMenuEdit.addSeparator();
    mMenuEdit.addAction(&mActions[ActionEditSelectAll]);
    mMenuEdit.addSeparator();
    mMenuEdit.addMenu(&mMenuTranspose);
    mMenuEdit.addAction(&mActions[ActionEditReverse]);
    mMenuEdit.addSeparator();
    mMenuEdit.addAction(&mActions[ActionEditKeyRepetition]);

    mMenuSong.setTitle(tr("&Song"));
    setupSongMenu(mMenuSong);

    mMenuView.setTitle(tr("&View"));
    mMenuViewToolbars.setTitle(tr("&Toolbars"));
    mMenuViewToolbars.addAction(mToolbarFile.toggleViewAction());
    mMenuViewToolbars.addAction(mToolbarEdit.toggleViewAction());
    mMenuViewToolbars.addAction(mToolbarSong.toggleViewAction());
    mMenuViewToolbars.addAction(mToolbarTracker.toggleViewAction());
    mMenuViewToolbars.addAction(mToolbarInput.toggleViewAction());
    mMenuViewToolbars.addAction(mToolbarInstrument.toggleViewAction());
    setupViewMenu(mMenuView);

    mMenuTracker.setTitle(tr("&Tracker"));
    mMenuTracker.addAction(&mActions[ActionTrackerPlay]);
    mMenuTracker.addAction(&mActions[ActionTrackerRestart]);
    mMenuTracker.addAction(&mActions[ActionTrackerPlayCurrentRow]);
    mMenuTracker.addAction(&mActions[ActionTrackerStepRow]);
    mMenuTracker.addAction(&mActions[ActionTrackerStop]);
    mMenuTracker.addAction(&mActions[ActionTrackerRepeat]);
    mMenuTracker.addAction(&mActions[ActionTrackerRecord]);
    mMenuTracker.addSeparator();

    mMenuTracker.addAction(&mActions[ActionTrackerToggleChannel]);
    mMenuTracker.addAction(&mActions[ActionTrackerSolo]);

    mMenuTracker.addSeparator();

    mMenuTracker.addAction(&mActions[ActionTrackerKill]);

    mMenuWindow.setTitle(tr("Wi&ndow"));
    //mMenuWindow is setup by setupWindowMenu()

    mMenuHelp.setTitle(tr("&Help"));
    mMenuHelp.addAction(&mActions[ActionHelpAudioDiag]);
    mMenuHelp.addSeparator();
    mMenuHelp.addAction(&mActions[ActionHelpAbout]);
    mMenuHelp.addAction(&mActions[ActionHelpAboutQt]);

    setupSongMenu(mContextMenuOrder);

    // MENUBAR ================================================================

    auto menubar = menuBar();
    menubar->addMenu(&mMenuFile);
    menubar->addMenu(&mMenuEdit);
    menubar->addMenu(&mMenuSong);
    menubar->addMenu(&mMenuTracker);
    menubar->addMenu(&mMenuView);
    menubar->addMenu(&mMenuWindow);
    menubar->addMenu(&mMenuHelp);

    // TOOLBARS ==============================================================

    QSize const iconSize(16, 16); // TODO: make this constant in IconManager

    mToolbarFile.setWindowTitle(tr("File"));
    mToolbarFile.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarFile);
    mToolbarFile.addAction(&mActions[ActionFileNew]);
    mToolbarFile.addAction(&mActions[ActionFileOpen]);
    mToolbarFile.addAction(&mActions[ActionFileSave]);
    mToolbarFile.addSeparator();
    mToolbarFile.addAction(&mActions[ActionFileConfig]);

    mToolbarEdit.setWindowTitle(tr("Edit"));
    mToolbarEdit.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarEdit);
    mToolbarEdit.addAction(mActionEditUndo);
    mToolbarEdit.addAction(mActionEditRedo);
    mToolbarEdit.addSeparator();
    mToolbarEdit.addAction(&mActions[ActionEditCut]);
    mToolbarEdit.addAction(&mActions[ActionEditCopy]);
    mToolbarEdit.addAction(&mActions[ActionEditPaste]);

    mToolbarSong.setWindowTitle(tr("Song"));
    mToolbarSong.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarSong);
    mToolbarSong.addAction(&mActions[ActionSongOrderInsert]);
    mToolbarSong.addAction(&mActions[ActionSongOrderRemove]);
    mToolbarSong.addAction(&mActions[ActionSongOrderMoveUp]);
    mToolbarSong.addAction(&mActions[ActionSongOrderMoveDown]);
    mToolbarSong.addAction(&mActions[ActionSongOrderDuplicate]);
    

    mToolbarTracker.setWindowTitle(tr("Tracker"));
    mToolbarTracker.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarTracker);
    mToolbarTracker.addAction(&mActions[ActionTrackerPlay]);
    mToolbarTracker.addAction(&mActions[ActionTrackerRestart]);
    mToolbarTracker.addAction(&mActions[ActionTrackerPlayCurrentRow]);
    mToolbarTracker.addAction(&mActions[ActionTrackerStepRow]);
    mToolbarTracker.addAction(&mActions[ActionTrackerStop]);
    mToolbarTracker.addSeparator();
    mToolbarTracker.addAction(&mActions[ActionTrackerRecord]);
    mToolbarTracker.addAction(&mActions[ActionTrackerRepeat]);
    mToolbarTracker.addAction(&mActions[ActionTrackerFollow]);
    
    mToolbarInput.setWindowTitle(tr("Input"));
    mToolbarInput.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarInput);
    mToolbarInput.addWidget(&mOctaveLabel);
    mToolbarInput.addWidget(&mOctaveSpin);
    mToolbarInput.addWidget(&mEditStepLabel);
    mToolbarInput.addWidget(&mEditStepSpin);
    mToolbarInput.addAction(&mActions[ActionEditKeyRepetition]);
    mToolbarInput.setStyleSheet(QStringLiteral("spacing: 8px;"));
    mOctaveSpin.setRange(2, 8);
    mOctaveSpin.setValue(mPianoInput.octave());
    mEditStepSpin.setRange(0, 255);
    mEditStepSpin.setValue(1);

    mToolbarInstrument.setWindowTitle(tr("Instrument"));
    mToolbarInstrument.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarInstrument);
    mToolbarInstrument.addWidget(&mInstrumentCombo);
    mInstrumentCombo.setMinimumWidth(200);
    mInstrumentCombo.setModel(&mInstrumentChoiceModel);

    // SHORTCUTS =============================================================

    mPlayAndStopShortcut.setKey(QKeySequence(Qt::Key_Return));
    mPlayAndStopShortcut.setContext(Qt::WidgetWithChildrenShortcut);

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
    connectActionToThis(mActions[ActionFileNew], onFileNew);
    connectActionToThis(mActions[ActionFileOpen], onFileOpen);
    connectActionToThis(mActions[ActionFileSave], onFileSave);
    connectActionToThis(mActions[ActionFileSaveAs], onFileSaveAs);
    connectActionToThis(mActions[ActionFileExportWav], showExportWavDialog);
    connectActionToThis(mActions[ActionFileClose], onFileClose);
    connectActionToThis(mActions[ActionFileCloseAll], onFileCloseAll);
    connectActionToThis(mActions[ActionFileConfig], showConfigDialog);
    connectActionToThis(mActions[ActionFileQuit], close);

    // Edit
    connect(&mActions[ActionEditCut], &QAction::triggered, &mPatternEditor, &PatternEditor::cut);
    connect(&mActions[ActionEditCopy], &QAction::triggered, &mPatternEditor, &PatternEditor::copy);
    connect(&mActions[ActionEditPaste], &QAction::triggered, &mPatternEditor, &PatternEditor::paste);
    connect(&mActions[ActionEditPasteMix], &QAction::triggered, &mPatternEditor, &PatternEditor::pasteMix);
    connect(&mActions[ActionEditErase], &QAction::triggered, &mPatternEditor, &PatternEditor::erase);
    connect(&mActions[ActionEditSelectAll], &QAction::triggered, &mPatternEditor, &PatternEditor::selectAll);
    connect(&mActions[ActionEditNoteIncrease], &QAction::triggered, &mPatternEditor, &PatternEditor::increaseNote);
    connect(&mActions[ActionEditNoteDecrease], &QAction::triggered, &mPatternEditor, &PatternEditor::decreaseNote);
    connect(&mActions[ActionEditOctaveIncrease], &QAction::triggered, &mPatternEditor, &PatternEditor::increaseOctave);
    connect(&mActions[ActionEditOctaveDecrease], &QAction::triggered, &mPatternEditor, &PatternEditor::decreaseOctave);
    connect(&mActions[ActionEditTranspose], &QAction::triggered, &mPatternEditor, &PatternEditor::transpose);
    connect(&mActions[ActionEditReverse], &QAction::triggered, &mPatternEditor, &PatternEditor::reverse);

    // view
    connectActionToThis(mActions[ActionViewResetLayout], onViewResetLayout);

    // window
    connectActionToThis(mActions[ActionWindowPrev], onWindowPrevious);
    connectActionToThis(mActions[ActionWindowNext], onWindowNext);

    // tracker
    connect(&mActions[ActionTrackerPlay], &QAction::triggered, mRenderer, &Renderer::play, Qt::QueuedConnection);
    connect(&mActions[ActionTrackerRestart], &QAction::triggered, mRenderer, &Renderer::playAtStart, Qt::QueuedConnection);
    connect(&mActions[ActionTrackerPlayCurrentRow], &QAction::triggered, mRenderer, &Renderer::playFromCursor, Qt::QueuedConnection);
    connect(&mActions[ActionTrackerStepRow], &QAction::triggered, mRenderer, &Renderer::stepFromCursor, Qt::QueuedConnection);
    connect(&mActions[ActionTrackerStop], &QAction::triggered, mRenderer, &Renderer::stopMusic, Qt::QueuedConnection);
    connect(&mActions[ActionTrackerKill], &QAction::triggered, mRenderer, &Renderer::forceStop, Qt::QueuedConnection);
    connect(&mActions[ActionTrackerRepeat], &QAction::toggled, mRenderer, &Renderer::setPatternRepeat, Qt::QueuedConnection);

    connect(&mActions[ActionTrackerToggleChannel], &QAction::triggered, this,
            [this]() {
                mBrowserModel.currentDocument()->toggleChannelOutput();
            });
    connect(&mActions[ActionTrackerSolo], &QAction::triggered, this,
            [this]() {
                mBrowserModel.currentDocument()->solo();
            });

    // help
    connectActionToThis(mActions[ActionHelpAudioDiag], showAudioDiag);
    connectActionToThis(mActions[ActionHelpAbout], showAboutDialog);
    QApplication::connect(&mActions[ActionHelpAboutQt], &QAction::triggered, &QApplication::aboutQt);

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

    connect(&mOctaveSpin, qOverload<int>(&QSpinBox::valueChanged), this, 
        [this](int octave) {
            mPianoInput.setOctave(octave);
        });
    connect(&mPatternEditor, &PatternEditor::changeOctave, &mOctaveSpin, &QSpinBox::setValue);

    connect(&mEditStepSpin, qOverload<int>(&QSpinBox::valueChanged), this,
            [this](int value) {
                mBrowserModel.currentDocument()->setEditStep(value);
            });

    connect(&mInstrumentCombo, qOverload<int>(&QComboBox::currentIndexChanged), &mPatternEditor, &PatternEditor::setInstrument);

    connect(&mPatternEditor, &PatternEditor::nextInstrument, this,
        [this]() {
            auto index = mInstrumentCombo.currentIndex() + 1;
            if (index < mInstrumentCombo.count()) {
                mInstrumentCombo.setCurrentIndex(index);
            }
        });
    connect(&mPatternEditor, &PatternEditor::previousInstrument, this,
        [this]() {
            auto index = mInstrumentCombo.currentIndex() - 1;
            if (index >= 0) {
                mInstrumentCombo.setCurrentIndex(index);
            }
        });

    connect(&mSidebar, &Sidebar::orderMenuRequested, this,
        [this](QPoint const& pos) {
            mContextMenuOrder.popup(pos);
        });

    // order actions
    connect(&mActions[ActionSongOrderInsert], &QAction::triggered, this,
        [this]() {
            mBrowserModel.currentDocument()->orderModel().insert();
            updateOrderActions();
        });
    connect(&mActions[ActionSongOrderRemove], &QAction::triggered, this,
        [this]() {
            mBrowserModel.currentDocument()->orderModel().remove();
            updateOrderActions();
        });
    connect(&mActions[ActionSongOrderDuplicate], &QAction::triggered, this,
        [this]() {
            mBrowserModel.currentDocument()->orderModel().duplicate();
            updateOrderActions();
        });
    connect(&mActions[ActionSongOrderMoveUp], &QAction::triggered, this,
        [this]() {
            mBrowserModel.currentDocument()->orderModel().moveUp();
            updateOrderActions();
        });
    connect(&mActions[ActionSongOrderMoveDown], &QAction::triggered, this,
        [this]() {
            mBrowserModel.currentDocument()->orderModel().moveDown();
            updateOrderActions();
        });

    connect(&mPatternEditor, &PatternEditor::previewNote, mRenderer, &Renderer::previewNote, Qt::QueuedConnection);
    connect(&mPatternEditor, &PatternEditor::stopNotePreview, mRenderer, &Renderer::stopPreview, Qt::QueuedConnection);
    

    connect(&mPlayAndStopShortcut, &QShortcut::activated, this,
        [this]() {
            if (mRenderer->isRunning()) {
                mActions[ActionTrackerStop].trigger();
            } else {
                mActions[ActionTrackerPlay].trigger();
            }
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
    connect(&mBrowserModel, &ModuleModel::currentDocumentChanged, &mSidebar, &Sidebar::setDocument);
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
    
    menu.addAction(&mActions[ActionViewResetLayout]);
}

void MainWindow::setupSongMenu(QMenu &menu) {
    menu.addAction(&mActions[ActionSongOrderInsert]);
    menu.addAction(&mActions[ActionSongOrderRemove]);
    menu.addAction(&mActions[ActionSongOrderDuplicate]);
    menu.addSeparator();
    menu.addAction(&mActions[ActionSongOrderMoveUp]);
    menu.addAction(&mActions[ActionSongOrderMoveDown]);
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

void MainWindow::updateOrderActions() {
    auto &model = mBrowserModel.currentDocument()->orderModel();
    bool canInsert = model.canInsert();
    mActions[ActionSongOrderInsert].setEnabled(canInsert);
    mActions[ActionSongOrderDuplicate].setEnabled(canInsert);
    mActions[ActionSongOrderRemove].setEnabled(model.canRemove());
    mActions[ActionSongOrderMoveUp].setEnabled(model.canMoveUp());
    mActions[ActionSongOrderMoveDown].setEnabled(model.canMoveDown());
}
