
#include "MainWindow.hpp"

#include "core/midi/MidiProber.hpp"
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

MainWindow::MainWindow() :
    QMainWindow(),
    mConfig(),
    mMidi(),
    mMidiReceiver(nullptr),
    mMidiNoteDown(false),
    mPianoInput(),
    mDocumentCounter(0),
    mDocument(),
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
    mMainWidget(nullptr),
    mEditorLayout(),
    mSidebar(mDocument),
    mPatternEditor(mDocument, mPianoInput),
    mPlayAndStopShortcut(&mPatternEditor),
    mRenderer(mDocument)
{
    mMidiReceiver = &mPatternEditor;

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

    // temporary
    mInstrumentEditor.setDocument(&mDocument);
    mWaveEditor.setDocument(&mDocument);
    mModuleSettingsWidget.setDocument(&mDocument);

    mDocument.setName(tr("Untitled 1"));
    updateWindowTitle();

    // apply the read in configuration
    onConfigApplied(Config::CategoryAll);

}

MainWindow::~MainWindow() {

}

QMenu* MainWindow::createPopupMenu() {
    // we can't return a reference to mMenuView as QMainWindow will delete it
    // a new menu must be created
    auto menu = new QMenu(this);
    setupViewMenu(*menu);
    return menu;
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    if (maybeSave()) {
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
    if (!maybeSave()) {
        return;
    }

    mRenderer.forceStop();

    ++mDocumentCounter;
    mDocument.clear();
    QString name = tr("Untitled %1").arg(mDocumentCounter);
    mDocument.setName(name);
    updateWindowTitle();

}

void MainWindow::onFileOpen() {
    if (!maybeSave()) {
        return;
    }
    
    auto path = QFileDialog::getOpenFileName(
        this,
        tr("Open module"),
        "",
        tr(MODULE_FILE_FILTER)
    );

    if (path.isEmpty()) {
        return;
    }

    mRenderer.forceStop();

    bool opened = mDocument.open(path);

    if (opened) {
        // update window title with document name
        updateWindowTitle();
    } else {
        auto error = mDocument.lastError();
        QMessageBox msgbox;
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("Could not open module"));
        switch (error) {
            case trackerboy::FormatError::duplicateId:
            case trackerboy::FormatError::invalid:
            case trackerboy::FormatError::tableDuplicateId:
            case trackerboy::FormatError::tableSizeBounds:
            case trackerboy::FormatError::unknownChannel:
                msgbox.setInformativeText(tr("The module is corrupted"));
                break;
            case trackerboy::FormatError::invalidSignature:
                msgbox.setInformativeText(tr("The file is not a trackerboy module"));
                break;
            default:
                msgbox.setInformativeText(tr("The file could not be read"));
                break;
            
        }

        msgbox.show();
        
    }

}

bool MainWindow::onFileSave() {
    if (mDocument.hasFile()) {
        return mDocument.save();
    } else {
        return onFileSaveAs();
    }
}

bool MainWindow::onFileSaveAs() {
     auto path = QFileDialog::getSaveFileName(
        this,
        tr("Save module"),
        "",
        tr(MODULE_FILE_FILTER)
        );

    if (path.isEmpty()) {
        return false;
    }

    auto result = mDocument.save(path);
    if (result) {
        // the document has a new name, update the window title
        updateWindowTitle();
    }
    return result;
}

void MainWindow::onViewResetLayout() {
    // remove everything
    removeToolBar(&mToolbarFile);
    removeToolBar(&mToolbarEdit);
    removeToolBar(&mToolbarTracker);

    initState();
}

void MainWindow::onConfigApplied(Config::Categories categories) {
    if (categories.testFlag(Config::CategorySound)) {
        auto &sound = mConfig.sound();
        auto samplerate = SAMPLERATE_TABLE[sound.samplerateIndex];
        mStatusSamplerate.setText(tr("%1 Hz").arg(samplerate));

        mErrorSinceLastConfig = !mRenderer.setConfig(sound);
        if (mErrorSinceLastConfig) {
            setPlayingStatus(PlayingStatusText::error);
            if (isVisible()) {
                QMessageBox msgbox(this);
                msgbox.setIcon(QMessageBox::Critical);
                msgbox.setText(tr("Could not initialize device"));
                msgbox.setInformativeText(tr("The configured device could not be initialized. Playback is disabled."));
                settingsMessageBox(msgbox);
            }
        } else {
            if (!mRenderer.isRunning()) {
                setPlayingStatus(PlayingStatusText::ready);
            }
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

    if (categories.testFlag(Config::CategoryMidi)) {
        auto midiConfig = mConfig.midi();
        auto &prober = MidiProber::instance();
        if (!midiConfig.enabled || midiConfig.portIndex == -1) {
            mMidi.close();
        } else {
            auto success = mMidi.setDevice(prober.backend(), midiConfig.portIndex);
            if (!success) {
                disableMidi(false);
            }
        }
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
        mAudioDiag = new AudioDiagDialog(mRenderer, this);
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
    auto dialog = new ExportWavDialog(mDocument, mConfig, this);
    dialog->show();
    dialog->exec();
    delete dialog;
}

void MainWindow::onAudioStart() {
    if (!mRenderer.isRunning()) {
        return;
    }

    mLastEngineFrame = {};
    mFrameSkip = 0;
    setPlayingStatus(PlayingStatusText::playing);
}

void MainWindow::onAudioError() {
    setPlayingStatus(PlayingStatusText::error);
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
    if (mRenderer.isRunning()) {
        return; // sometimes it takes too long for this signal to get here
    }

    if (!mErrorSinceLastConfig) {
        setPlayingStatus(PlayingStatusText::ready);
    }
}

void MainWindow::onFrameSync() {
    // this slot is called when the renderer has renderered a new frame
    // sync is a bit misleading here, as this slot is called when this frame
    // is in process of being bufferred. It is not the current frame being played out.

    auto frame = mRenderer.currentFrame();

    // check if the player position changed
    if (frame.startedNewRow) {        
        // update tracker position
        mDocument.patternModel().setTrackerCursor(frame.row, frame.order);

        // update position status
        mStatusPos.setText(QStringLiteral("%1 / %2")
            .arg(frame.order, 2, 10, QChar('0'))
            .arg(frame.row, 2, 10, QChar('0')));
    }

    // check if the speed changed
    if (mLastEngineFrame.speed != frame.speed) {
        auto speedF = trackerboy::speedToFloat(frame.speed);
        // update speed status
        mStatusSpeed.setText(tr("%1 FPR").arg(speedF, 0, 'f', 3));
        auto tempo = trackerboy::speedToTempo(speedF, mDocument.songModel().rowsPerBeat());
        mStatusTempo.setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
    }

    constexpr auto FRAME_SKIP = 30;

    if (mLastEngineFrame.time != frame.time) {
        // determine elapsed time
        if (mFrameSkip == 0) {

            auto framerate = mDocument.framerate();
            int elapsed = frame.time / framerate;
            int secs = elapsed;
            int mins = secs / 60;
            secs = secs % 60;

            QString str = QStringLiteral("%1:%2")
                .arg(mins, 2, 10, QChar('0'))
                .arg(secs, 2, 10, QChar('0'));
            mStatusElapsed.setText(str);


            mFrameSkip = FRAME_SKIP;
        } else {
            --mFrameSkip;
        }
    }

    mLastEngineFrame = frame;
}

#if 0
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
#endif

// PRIVATE METHODS -----------------------------------------------------------

bool MainWindow::maybeSave() {
    if (mDocument.isModified()) {
        // prompt the user if they want to save any changes
        auto const result = QMessageBox::warning(
            this,
            tr("Trackerboy"),
            tr("Save changes to %1?").arg(mDocument.name()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        switch (result) {
            case QMessageBox::Save:
                if (!onFileSave()) {
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

    return true;
}

void MainWindow::setupUi() {

    // CENTRAL WIDGET ========================================================

    // MainWindow expects this to heap-alloc'd as it will manually delete the widget
    mMainWidget = new QWidget(this);
    mMainWidget->setObjectName(QStringLiteral("MainWidget"));

    //mEditorLayout.addLayout(&mGroupLayout);
    mEditorLayout.addWidget(&mSidebar);
    mEditorLayout.addWidget(&mPatternEditor, 1);
    mEditorLayout.setMargin(0);
    mMainWidget->setLayout(&mEditorLayout);

    setCentralWidget(mMainWidget);

    mSidebar.scope().setBuffer(&mRenderer.visualizerBuffer());

    // ACTIONS ===============================================================

    setupAction(mActions[ActionFileNew], "&New", "Create a new module", Icons::fileNew, QKeySequence::New);
    setupAction(mActions[ActionFileOpen], "&Open", "Open an existing module", Icons::fileOpen, QKeySequence::Open);
    setupAction(mActions[ActionFileSave], "&Save", "Save the module", Icons::fileSave, QKeySequence::Save);
    setupAction(mActions[ActionFileSaveAs], "Save &As...", "Save the module to a new file", QKeySequence::SaveAs);
    setupAction(mActions[ActionFileExportWav], "Export to WAV...", "Exports the module to a WAV file");
    setupAction(mActions[ActionFileConfig], "&Configuration...", "Change application settings", Icons::fileConfig);
    setupAction(mActions[ActionFileQuit], "&Quit", "Exit the application", QKeySequence::Quit);

    auto &undoStack = mDocument.undoStack();
    mActionEditUndo = undoStack.createUndoAction(this);
    mActionEditUndo->setIcon(IconManager::getIcon(Icons::editUndo));
    mActionEditUndo->setShortcut(QKeySequence::Undo);
    mActionEditRedo = undoStack.createRedoAction(this);
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
    mActions[ActionTrackerFollow].setChecked(true);
    
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
    menubar->addMenu(&mMenuHelp);

    // TOOLBARS ==============================================================

    QSize const iconSize = IconManager::size();

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
    mToolbarInput.addSeparator();
    mToolbarInput.addAction(&mActions[ActionEditKeyRepetition]);
    mOctaveSpin.setRange(2, 8);
    mOctaveSpin.setValue(mPianoInput.octave());
    mEditStepSpin.setRange(0, 255);
    mEditStepSpin.setValue(mDocument.editStep());

    mToolbarInstrument.setWindowTitle(tr("Instrument"));
    mToolbarInstrument.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarInstrument);
    mToolbarInstrument.addWidget(&mInstrumentCombo);
    mInstrumentCombo.setMinimumWidth(200);
    mInstrumentCombo.setModel(&mInstrumentChoiceModel);
    mInstrumentChoiceModel.setModel(&mDocument.instrumentModel());

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
    mUndoView.setStack(&undoStack);

    // STATUSBAR ==============================================================

    auto statusbar = statusBar();

    mStatusRenderer.setMinimumWidth(60);
    mStatusSpeed.setMinimumWidth(60);
    mStatusTempo.setMinimumWidth(60);
    mStatusElapsed.setMinimumWidth(40);
    mStatusPos.setMinimumWidth(40);
    mStatusSamplerate.setMinimumWidth(60);

    {
        for (auto label : { &mStatusRenderer, &mStatusSpeed, &mStatusTempo, &mStatusElapsed, &mStatusPos, &mStatusSamplerate }) {
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    
    statusbar->addPermanentWidget(&mStatusRenderer);
    statusbar->addPermanentWidget(&mStatusSpeed);
    statusbar->addPermanentWidget(&mStatusTempo);
    statusbar->addPermanentWidget(&mStatusElapsed);
    statusbar->addPermanentWidget(&mStatusPos);
    statusbar->addPermanentWidget(&mStatusSamplerate);

    statusbar->showMessage(tr("Trackerboy v%1.%2.%3")
        .arg(trackerboy::VERSION.major)
        .arg(trackerboy::VERSION.minor)
        .arg(trackerboy::VERSION.patch));

    // default statuses
    setPlayingStatus(PlayingStatusText::ready);
    mStatusSpeed.setText(tr("6.000 FPR"));
    mStatusTempo.setText(tr("150 BPM"));
    mStatusElapsed.setText(QStringLiteral("00:00"));
    mStatusPos.setText(QStringLiteral("00 / 00"));
    // no need to set samplerate, it is done so in onConfigApplied

    // CONNECTIONS ============================================================

    // Actions

    // File
    connectActionToThis(mActions[ActionFileNew], onFileNew);
    connectActionToThis(mActions[ActionFileOpen], onFileOpen);
    connectActionToThis(mActions[ActionFileSave], onFileSave);
    connectActionToThis(mActions[ActionFileSaveAs], onFileSaveAs);
    connectActionToThis(mActions[ActionFileExportWav], showExportWavDialog);
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

    // tracker
    connect(&mActions[ActionTrackerPlay], &QAction::triggered, &mRenderer, &Renderer::play, Qt::DirectConnection);
    connect(&mActions[ActionTrackerRestart], &QAction::triggered, &mRenderer, &Renderer::playAtStart, Qt::DirectConnection);
    connect(&mActions[ActionTrackerPlayCurrentRow], &QAction::triggered, &mRenderer, &Renderer::playFromCursor, Qt::DirectConnection);
    connect(&mActions[ActionTrackerStepRow], &QAction::triggered, &mRenderer, &Renderer::stepFromCursor, Qt::DirectConnection);
    connect(&mActions[ActionTrackerStop], &QAction::triggered, &mRenderer, &Renderer::stopMusic, Qt::DirectConnection);
    connect(&mActions[ActionTrackerKill], &QAction::triggered, &mRenderer, &Renderer::forceStop, Qt::DirectConnection);
    connect(&mActions[ActionTrackerRepeat], &QAction::toggled, &mRenderer, &Renderer::setPatternRepeat, Qt::DirectConnection);

    connect(&mActions[ActionTrackerToggleChannel], &QAction::triggered, &mDocument, &ModuleDocument::toggleChannelOutput);
    connect(&mActions[ActionTrackerSolo], &QAction::triggered, &mDocument, &ModuleDocument::solo);

    // help
    connectActionToThis(mActions[ActionHelpAudioDiag], showAudioDiag);
    connectActionToThis(mActions[ActionHelpAbout], showAboutDialog);
    QApplication::connect(&mActions[ActionHelpAboutQt], &QAction::triggered, &QApplication::aboutQt);

    // editors
    {
        auto &piano = mInstrumentEditor.piano();
        connect(&piano, &PianoWidget::keyDown, &mRenderer, &Renderer::previewInstrument, Qt::DirectConnection);
        connect(&piano, &PianoWidget::keyUp, &mRenderer, &Renderer::stopPreview, Qt::DirectConnection);
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
        connect(&piano, &PianoWidget::keyDown, &mRenderer, &Renderer::previewWaveform, Qt::DirectConnection);
        connect(&piano, &PianoWidget::keyUp, &mRenderer, &Renderer::stopPreview, Qt::DirectConnection);
    }

    connect(&mOctaveSpin, qOverload<int>(&QSpinBox::valueChanged), this, 
        [this](int octave) {
            mPianoInput.setOctave(octave);
        });
    connect(&mPatternEditor, &PatternEditor::changeOctave, &mOctaveSpin, &QSpinBox::setValue);

    connect(&mEditStepSpin, qOverload<int>(&QSpinBox::valueChanged), &mDocument, &ModuleDocument::setEditStep);
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

    connect(&mSidebar, &Sidebar::patternJumpRequested, &mRenderer, &Renderer::jumpToPattern);

    // order actions
    connect(&mActions[ActionSongOrderInsert], &QAction::triggered, this,
        [this]() {
            mDocument.orderModel().insert();
            updateOrderActions();
        });
    connect(&mActions[ActionSongOrderRemove], &QAction::triggered, this,
        [this]() {
            mDocument.orderModel().remove();
            updateOrderActions();
        });
    connect(&mActions[ActionSongOrderDuplicate], &QAction::triggered, this,
        [this]() {
            mDocument.orderModel().duplicate();
            updateOrderActions();
        });
    connect(&mActions[ActionSongOrderMoveUp], &QAction::triggered, this,
        [this]() {
            mDocument.orderModel().moveUp();
            updateOrderActions();
        });
    connect(&mActions[ActionSongOrderMoveDown], &QAction::triggered, this,
        [this]() {
            mDocument.orderModel().moveDown();
            updateOrderActions();
        });

    connect(&mPatternEditor, &PatternEditor::previewNote, &mRenderer, &Renderer::previewNote, Qt::DirectConnection);
    connect(&mPatternEditor, &PatternEditor::stopNotePreview, &mRenderer, &Renderer::stopPreview, Qt::DirectConnection);
    

    connect(&mPlayAndStopShortcut, &QShortcut::activated, this,
        [this]() {
            if (mRenderer.isRunning()) {
                mActions[ActionTrackerStop].trigger();
            } else {
                mActions[ActionTrackerPlay].trigger();
            }
        });

    connect(&mMidi, &Midi::error, this,
        [this]() {
            disableMidi(true);
        });

    connect(&mMidi, &Midi::noteOn, this,
        [this](int note) {
            mMidiReceiver->midiNoteOn(note);
            mMidiNoteDown = true;
        });

    connect(&mMidi, &Midi::noteOff, this,
        [this]() {
            mMidiReceiver->midiNoteOff();
            mMidiNoteDown = false;
        });

    connect(&mDocument, &ModuleDocument::modifiedChanged, this, &MainWindow::setWindowModified);
    connect(&mActions[ActionEditKeyRepetition], &QAction::toggled, &mDocument, &ModuleDocument::setKeyRepetition);

    auto &patternModel = mDocument.patternModel();
    connect(&patternModel, &PatternModel::recordingChanged, &mActions[ActionTrackerRecord], &QAction::setChecked);
    connect(&mActions[ActionTrackerRecord], &QAction::toggled, &patternModel, &PatternModel::setRecord);
    connect(&mActions[ActionTrackerFollow], &QAction::toggled, &patternModel, &PatternModel::setFollowing);

    auto &orderModel = mDocument.orderModel();
    connect(&orderModel, &OrderModel::currentPatternChanged, this, &MainWindow::updateOrderActions);
    updateOrderActions();

    connect(&mRenderer, &Renderer::audioStarted, this, &MainWindow::onAudioStart);
    connect(&mRenderer, &Renderer::audioStopped, this, &MainWindow::onAudioStop);
    connect(&mRenderer, &Renderer::audioError, this, &MainWindow::onAudioError);
    connect(&mRenderer, &Renderer::frameSync, this, &MainWindow::onFrameSync);
    
    connect(&mRenderer, &Renderer::updateVisualizers, &mSidebar.scope(), qOverload<>(&AudioScope::update));

    auto app = static_cast<QApplication*>(QApplication::instance());
    connect(app, &QApplication::focusChanged, this, &MainWindow::handleFocusChange);

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
    setWindowTitle(QStringLiteral("%1[*] - Trackerboy").arg(mDocument.name()));
}

void MainWindow::updateOrderActions() {
    auto &model = mDocument.orderModel();
    bool canInsert = model.canInsert();
    mActions[ActionSongOrderInsert].setEnabled(canInsert);
    mActions[ActionSongOrderDuplicate].setEnabled(canInsert);
    mActions[ActionSongOrderRemove].setEnabled(model.canRemove());
    mActions[ActionSongOrderMoveUp].setEnabled(model.canMoveUp());
    mActions[ActionSongOrderMoveDown].setEnabled(model.canMoveDown());
}

void MainWindow::setPlayingStatus(PlayingStatusText type) {
    static const char *PLAYING_STATUSES[] = {
        QT_TR_NOOP("Ready"),
        QT_TR_NOOP("Playing"),
        QT_TR_NOOP("Device error")
    };

    mStatusRenderer.setText(tr(PLAYING_STATUSES[(int)type]));
}

void MainWindow::disableMidi(bool causedByError) {
    mConfig.disableMidi();
    if (mConfigDialog) {
        mConfigDialog->resetControls();
    }

    if (!causedByError) {
        qCritical().noquote() << "[MIDI] Failed to initialize MIDI device:" << mMidi.lastErrorString();
    }

    if (isVisible()) {
        QMessageBox msgbox(this);
        msgbox.setIcon(QMessageBox::Critical);
        if (causedByError) {
            msgbox.setText(tr("MIDI device error"));
        } else {
            msgbox.setText(tr("Could not initialize MIDI device"));
        }
        msgbox.setInformativeText(mMidi.lastErrorString());
        settingsMessageBox(msgbox);
    }
}

void MainWindow::handleFocusChange(QWidget *oldWidget, QWidget *newWidget) {
    Q_UNUSED(oldWidget)

    // this handler is for determining where MIDI events will go, based on
    // who has focus. If MIDI is disabled we don't need to do anything

    // performance notes
    // searching is required by walking the newWidget's parents until either
    // nullptr or the editor dock widgets are found. Performance depends on
    // how deep newWidget is nested in the hierarchy.

    // alternative solution:
    // BaseEditor could have an eventFilter installed on all of its child widgets,
    // that check for FocusIn and FocusOut events, which we could emit a
    // signal for these and the MainWindow would handle them appropriately.

    if (mMidi.isEnabled()) {
        if (QApplication::activeModalWidget()) {
            return; // ignore if a dialog is open
        }

        // MIDI events by default go to the pattern editor
        IMidiReceiver *receiver = &mPatternEditor;

        QWidget *widget = newWidget;
        while (widget) {
            // search if this widget's parent is the instrument or waveform editor dock
            // if it is, midi events will go to the editor's piano widget
            if (widget == &mDockWaveformEditor) {
                receiver = &mWaveEditor.piano();
                break;
            } else if (widget == &mDockInstrumentEditor) {
                receiver = &mInstrumentEditor.piano();
                break;
            }
            widget = widget->parentWidget();
        }

        if (mMidiReceiver != receiver) {
            // change the receiver
            if (mMidiNoteDown) {
                // force the note off
                // if we don't do this, the previous receiver won't get the next noteOff message
                // and the note will be held indefinitely
                mMidiReceiver->midiNoteOff();
                mMidiNoteDown = false;
            }
            mMidiReceiver = receiver;

        }
    }

}
