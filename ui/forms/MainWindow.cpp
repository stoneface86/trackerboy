
#include "MainWindow.hpp"

#include "core/midi/MidiProber.hpp"
#include "core/samplerates.hpp"
//#include "forms/ExportWavDialog.hpp"
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

MainWindow::MainWindow() :
    QMainWindow(),
    mUntitledString(tr("Untitled")),
    mConfig(),
    mMidi(),
    mMidiReceiver(nullptr),
    mMidiNoteDown(false),
    mPianoInput(),
    mModule(),
    mModuleFile(),
    //mRenderer(mDocument),
    mErrorSinceLastConfig(false),
    mAboutDialog(nullptr),
    mAudioDiag(nullptr),
    mConfigDialog(nullptr),
    mToolbarFile(),
    mToolbarEdit(),
    mToolbarTracker(),
    mToolbarInput(),
    mToolbarInstrument(),
    mInstrumentCombo(),
    // mDockInstrumentEditor(),
    // mInstrumentEditor(mPianoInput),
    // mDockWaveformEditor(),
    // mWaveEditor(mPianoInput),
    mDockHistory(),
    mUndoView()
    // mMainWidget(nullptr),
    // mEditorLayout(),
    // mSidebar(mDocument),
    // mPatternEditor(mDocument, mPianoInput),
    // mPlayAndStopShortcut(&mPatternEditor)
{
    //mMidiReceiver = &mPatternEditor;

    // create models
    mModule = new Module(this);
    mInstrumentModel = new InstrumentListModel(*mModule, this);
    mOrderModel = new OrderModel(*mModule, this);
    mSongModel = new SongModel(*mModule, this);
    mPatternModel = new PatternModel(*mModule, *mOrderModel, *mSongModel, this);
    mWaveModel = new WaveListModel(*mModule, this);

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
        addToolBar(mToolbarFile);
        addToolBar(mToolbarEdit);
        addToolBar(mToolbarSong);
        addToolBar(mToolbarTracker);
        addToolBar(mToolbarInput);
        addToolBar(mToolbarInstrument);
        // addDockWidget(Qt::LeftDockWidgetArea, &mDockModuleSettings);
        // addDockWidget(Qt::LeftDockWidgetArea, &mDockInstrumentEditor);
        // addDockWidget(Qt::LeftDockWidgetArea, &mDockWaveformEditor);
        addDockWidget(Qt::LeftDockWidgetArea, &mDockHistory);
        restoreState(windowState);
    }

    // temporary
    // mInstrumentEditor.setDocument(&mDocument);
    // mWaveEditor.setDocument(&mDocument);
    // mModuleSettingsWidget.setDocument(&mDocument);

    mModuleFile.setName(mUntitledString);
    updateWindowTitle();

    // apply the read in configuration
    onConfigApplied(Config::CategoryAll);

}

MainWindow::~MainWindow() {

}

QMenu* MainWindow::createPopupMenu() {
    auto menu = new QMenu(this);
    setupViewMenu(menu);
    return menu;
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    if (maybeSave()) {
        // user saved or discarded changes, close the window
        #ifdef QT_DEBUG
        if (mSaveConfig) {
        #endif
            QSettings settings;
            settings.setValue("geometry", saveGeometry());
            settings.setValue("windowState", saveState());
        #ifdef QT_DEBUG
        }
        #endif
        evt->accept();
    } else {
        // user aborted closing, ignore this event
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

    //mRenderer.forceStop();

    mModule->clear();

    mModuleFile.setName(mUntitledString);
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

    //mRenderer.forceStop();

    bool opened = mModuleFile.open(path, *mModule);

    if (!opened) {
        QMessageBox msgbox;
        msgbox.setIcon(QMessageBox::Critical);
        msgbox.setText(tr("Could not open module"));
        
        auto error = mModuleFile.lastError();
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
        

        msgbox.exec();
        mModuleFile.setName(mUntitledString);
        
    }

    // update window title with document name
    updateWindowTitle();

}

bool MainWindow::onFileSave() {
    if (mModuleFile.hasFile()) {
        return mModuleFile.save(*mModule);
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

    auto result = mModuleFile.save(path, *mModule);
    if (result) {
        // the document has a new name, update the window title
        updateWindowTitle();
    }
    return result;
}

void MainWindow::onSongOrderInsert() {
    mOrderModel->insert();
    updateOrderActions();
}

void MainWindow::onSongOrderRemove() {
    mOrderModel->remove();
    updateOrderActions();
}

void MainWindow::onSongOrderDuplicate() {
    mOrderModel->duplicate();
    updateOrderActions();
}

void MainWindow::onSongOrderMoveUp() {
    mOrderModel->moveUp();
    updateOrderActions();
}

void MainWindow::onSongOrderMoveDown() {
    mOrderModel->moveDown();
    updateOrderActions();
}

void MainWindow::onViewResetLayout() {
    // remove everything
    removeToolBar(mToolbarFile);
    removeToolBar(mToolbarEdit);
    removeToolBar(mToolbarTracker);
    removeToolBar(mToolbarInput);
    removeToolBar(mToolbarInstrument);

    initState();
}

void MainWindow::onConfigApplied(Config::Categories categories) {
    if (categories.testFlag(Config::CategorySound)) {
        auto &sound = mConfig.sound();
        auto samplerate = SAMPLERATE_TABLE[sound.samplerateIndex];
        mStatusSamplerate.setText(tr("%1 Hz").arg(samplerate));

        // mErrorSinceLastConfig = !mRenderer.setConfig(sound);
        // if (mErrorSinceLastConfig) {
        //     setPlayingStatus(PlayingStatusText::error);
        //     if (isVisible()) {
        //         QMessageBox msgbox(this);
        //         msgbox.setIcon(QMessageBox::Critical);
        //         msgbox.setText(tr("Could not initialize device"));
        //         msgbox.setInformativeText(tr("The configured device could not be initialized. Playback is disabled."));
        //         settingsMessageBox(msgbox);
        //     }
        // } else {
        //     if (!mRenderer.isRunning()) {
        //         setPlayingStatus(PlayingStatusText::ready);
        //     }
        // }
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

        //mPatternEditor.setColors(appearance.colors);
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
    // if (mAudioDiag == nullptr) {
    //     mAudioDiag = new AudioDiagDialog(mRenderer, this);
    // }

    // mAudioDiag->show();
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
    // auto dialog = new ExportWavDialog(mDocument, mConfig, this);
    // dialog->show();
    // dialog->exec();
    // delete dialog;
}

void MainWindow::onAudioStart() {
    // if (!mRenderer.isRunning()) {
    //     return;
    // }

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
    // if (mRenderer.isRunning()) {
    //     return; // sometimes it takes too long for this signal to get here
    // }

    if (!mErrorSinceLastConfig) {
        setPlayingStatus(PlayingStatusText::ready);
    }
}

void MainWindow::onFrameSync() {
    // this slot is called when the renderer has renderered a new frame
    // sync is a bit misleading here, as this slot is called when this frame
    // is in process of being bufferred. It is not the current frame being played out.

    // auto frame = mRenderer.currentFrame();

    // // check if the player position changed
    // if (frame.startedNewRow) {        
    //     // update tracker position
    //     mDocument.patternModel().setTrackerCursor(frame.row, frame.order);

    //     // update position status
    //     mStatusPos.setText(QStringLiteral("%1 / %2")
    //         .arg(frame.order, 2, 10, QChar('0'))
    //         .arg(frame.row, 2, 10, QChar('0')));
    // }

    // // check if the speed changed
    // if (mLastEngineFrame.speed != frame.speed) {
    //     auto speedF = trackerboy::speedToFloat(frame.speed);
    //     // update speed status
    //     mStatusSpeed.setText(tr("%1 FPR").arg(speedF, 0, 'f', 3));
    //     auto tempo = trackerboy::speedToTempo(speedF, mDocument.songModel().rowsPerBeat());
    //     mStatusTempo.setText(tr("%1 BPM").arg(tempo, 0, 'f', 2));
    // }

    // constexpr auto FRAME_SKIP = 30;

    // if (mLastEngineFrame.time != frame.time) {
    //     // determine elapsed time
    //     if (mFrameSkip == 0) {

    //         auto framerate = mDocument.framerate();
    //         int elapsed = frame.time / framerate;
    //         int secs = elapsed;
    //         int mins = secs / 60;
    //         secs = secs % 60;

    //         QString str = QStringLiteral("%1:%2")
    //             .arg(mins, 2, 10, QChar('0'))
    //             .arg(secs, 2, 10, QChar('0'));
    //         mStatusElapsed.setText(str);


    //         mFrameSkip = FRAME_SKIP;
    //     } else {
    //         --mFrameSkip;
    //     }
    // }

    // mLastEngineFrame = frame;
}

// PRIVATE METHODS -----------------------------------------------------------

bool MainWindow::maybeSave() {
    if (mModule->isModified()) {
        // prompt the user if they want to save any changes
        auto const result = QMessageBox::warning(
            this,
            tr("Trackerboy"),
            tr("Save changes to %1?").arg(mModuleFile.name()),
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


QToolBar* MainWindow::makeToolbar(QString const& title, QString const& objname) {
    auto toolbar = new QToolBar(title, this);
    toolbar->setObjectName(objname);
    toolbar->setIconSize(IconManager::size());
    return toolbar;
}

void MainWindow::setupUi() {

    // CENTRAL WIDGET ========================================================

    auto centralWidget = new QWidget(this);
    auto layout = new QHBoxLayout;
    mSidebar = new Sidebar;
    layout->addWidget(mSidebar);
    layout->addStretch(1); // temporary
    centralWidget->setLayout(layout);

    mSidebar->orderEditor()->setModel(mOrderModel);
    mSidebar->songEditor()->setModel(mSongModel);

    setCentralWidget(centralWidget);

    // mMainWidget = new QWidget(this);
    // mMainWidget->setObjectName(QStringLiteral("MainWidget"));

    // //mEditorLayout.addLayout(&mGroupLayout);
    // mEditorLayout.addWidget(&mSidebar);
    // mEditorLayout.addWidget(&mPatternEditor, 1);
    // mEditorLayout.setMargin(0);
    // mMainWidget->setLayout(&mEditorLayout);

    // setCentralWidget(mMainWidget);

    // mSidebar.scope().setBuffer(&mRenderer.visualizerBuffer());


    // TOOLBARS ==============================================================

    mToolbarFile = makeToolbar(tr("File"), QStringLiteral("ToolbarFile"));
    mToolbarEdit = makeToolbar(tr("Edit"), QStringLiteral("ToolbarEdit"));
    mToolbarSong = makeToolbar(tr("Song"), QStringLiteral("ToolbarSong"));
    mToolbarTracker = makeToolbar(tr("Tracker"), QStringLiteral("ToolbarTracker"));
    mToolbarInput = makeToolbar(tr("Input"), QStringLiteral("ToolbarInput"));
    mToolbarInstrument = makeToolbar(tr("Instrument"), QStringLiteral("ToolbarInstrument"));


    mOctaveSpin = new QSpinBox(mToolbarInput);
    mOctaveSpin->setRange(2, 8);
    mOctaveSpin->setValue(mPianoInput.octave());
    auto editStepSpin = new QSpinBox(mToolbarInput);
    editStepSpin->setRange(0, 255);
    editStepSpin->setValue(1);
    mToolbarInput->addWidget(new QLabel(tr("Octave"), mToolbarInput));
    mToolbarInput->addWidget(mOctaveSpin);
    mToolbarInput->addWidget(new QLabel(tr("Edit step"), mToolbarInput));
    mToolbarInput->addWidget(editStepSpin);
    mToolbarInput->addSeparator();

    mInstrumentCombo = new QComboBox(mToolbarInstrument);
    auto model = new InstrumentChoiceModel(mInstrumentCombo);
    model->setModel(mInstrumentModel);
    mInstrumentCombo->setMinimumWidth(200);
    mInstrumentCombo->setModel(model);
    mToolbarInstrument->addWidget(mInstrumentCombo);

    // ACTIONS ===============================================================

    createActions();

    mToolbarSong->addAction(mActionOrderInsert);
    mToolbarSong->addAction(mActionOrderRemove);
    mToolbarSong->addAction(mActionOrderDuplicate);
    mToolbarSong->addAction(mActionOrderMoveUp);
    mToolbarSong->addAction(mActionOrderMoveDown);

    // SHORTCUTS =============================================================

    // mPlayAndStopShortcut.setKey(QKeySequence(Qt::Key_Return));
    // mPlayAndStopShortcut.setContext(Qt::WidgetWithChildrenShortcut);

    // DOCKS =================================================================

    // setObjectNameFromDeclared(mDockModuleSettings);
    // mDockModuleSettings.setWindowTitle(tr("Module settings"));
    // mDockModuleSettings.setWidget(&mModuleSettingsWidget);
    
    // setObjectNameFromDeclared(mDockInstrumentEditor);
    // mDockInstrumentEditor.setWindowTitle(tr("Instrument editor"));
    // mDockInstrumentEditor.setWidget(&mInstrumentEditor);
    
    // setObjectNameFromDeclared(mDockWaveformEditor);
    // mDockWaveformEditor.setWindowTitle(tr("Waveform editor"));
    // mDockWaveformEditor.setWidget(&mWaveEditor);

    setObjectNameFromDeclared(mDockHistory);
    mDockHistory.setWindowTitle(tr("History"));
    mDockHistory.setWidget(&mUndoView);
    mUndoView.setStack(&mModule->undoStack());

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

    // editors
    // {
    //     auto &piano = mInstrumentEditor.piano();
    //     connect(&piano, &PianoWidget::keyDown, &mRenderer, &Renderer::previewInstrument, Qt::DirectConnection);
    //     connect(&piano, &PianoWidget::keyUp, &mRenderer, &Renderer::stopPreview, Qt::DirectConnection);
    // }

    // connect(&mInstrumentEditor, &InstrumentEditor::openWaveEditor, this,
    //     [this](int index) {
    //         mWaveEditor.openItem(index);
    //         mDockWaveformEditor.show();
    //         mDockWaveformEditor.raise();
    //         mDockWaveformEditor.activateWindow();
    //     });

    // {
    //     auto &piano = mWaveEditor.piano();
    //     connect(&piano, &PianoWidget::keyDown, &mRenderer, &Renderer::previewWaveform, Qt::DirectConnection);
    //     connect(&piano, &PianoWidget::keyUp, &mRenderer, &Renderer::stopPreview, Qt::DirectConnection);
    // }

    connect(mOctaveSpin, qOverload<int>(&QSpinBox::valueChanged), this, 
        [this](int octave) {
            mPianoInput.setOctave(octave);
        });
    // connect(&mPatternEditor, &PatternEditor::changeOctave, &mOctaveSpin, &QSpinBox::setValue);

    // connect(&mEditStepSpin, qOverload<int>(&QSpinBox::valueChanged), &mDocument, &ModuleDocument::setEditStep);
    // connect(&mInstrumentCombo, qOverload<int>(&QComboBox::currentIndexChanged), &mPatternEditor, &PatternEditor::setInstrument);

    // connect(&mPatternEditor, &PatternEditor::nextInstrument, this,
    //     [this]() {
    //         auto index = mInstrumentCombo.currentIndex() + 1;
    //         if (index < mInstrumentCombo.count()) {
    //             mInstrumentCombo.setCurrentIndex(index);
    //         }
    //     });
    // connect(&mPatternEditor, &PatternEditor::previousInstrument, this,
    //     [this]() {
    //         auto index = mInstrumentCombo.currentIndex() - 1;
    //         if (index >= 0) {
    //             mInstrumentCombo.setCurrentIndex(index);
    //         }
    //     });

    connect(mSidebar->orderEditor(), &OrderEditor::popupMenuAt, this,
        [this](QPoint const& pos) {
            if (mSongOrderContextMenu == nullptr) {
                mSongOrderContextMenu = new QMenu(this);
                setupSongMenu(mSongOrderContextMenu);
            }
            mSongOrderContextMenu->popup(pos);
        });

    // connect(&mSidebar, &Sidebar::patternJumpRequested, &mRenderer, &Renderer::jumpToPattern);


    // connect(&mPatternEditor, &PatternEditor::previewNote, &mRenderer, &Renderer::previewNote, Qt::DirectConnection);
    // connect(&mPatternEditor, &PatternEditor::stopNotePreview, &mRenderer, &Renderer::stopPreview, Qt::DirectConnection);
    

    // connect(&mPlayAndStopShortcut, &QShortcut::activated, this,
    //     [this]() {
    //         if (mRenderer.isRunning()) {
    //             mActions[ActionTrackerStop].trigger();
    //         } else {
    //             mActions[ActionTrackerPlay].trigger();
    //         }
    //     });

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

    connect(mModule, &Module::modifiedChanged, this, &MainWindow::setWindowModified);
    // connect(&mActions[ActionEditKeyRepetition], &QAction::toggled, &mDocument, &ModuleDocument::setKeyRepetition);

    connect(mOrderModel, &OrderModel::currentPatternChanged, this, &MainWindow::updateOrderActions);
    updateOrderActions();

    // connect(&mRenderer, &Renderer::audioStarted, this, &MainWindow::onAudioStart);
    // connect(&mRenderer, &Renderer::audioStopped, this, &MainWindow::onAudioStop);
    // connect(&mRenderer, &Renderer::audioError, this, &MainWindow::onAudioError);
    // connect(&mRenderer, &Renderer::frameSync, this, &MainWindow::onFrameSync);
    
    // connect(&mRenderer, &Renderer::updateVisualizers, &mSidebar.scope(), qOverload<>(&AudioScope::update));

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


    addToolBar(Qt::TopToolBarArea, mToolbarFile);
    mToolbarFile->show();

    addToolBar(Qt::TopToolBarArea, mToolbarEdit);
    mToolbarEdit->show();

    addToolBar(Qt::TopToolBarArea, mToolbarSong);
    mToolbarSong->show();

    addToolBar(Qt::TopToolBarArea, mToolbarTracker);
    mToolbarTracker->show();

    addToolBar(Qt::TopToolBarArea, mToolbarInput);
    mToolbarInput->show();

    addToolBar(Qt::TopToolBarArea, mToolbarInstrument);
    mToolbarInstrument->show();

    // addDockWidget(Qt::RightDockWidgetArea, &mDockModuleSettings);
    // mDockModuleSettings.setFloating(true);
    // mDockModuleSettings.hide();

    // addDockWidget(Qt::RightDockWidgetArea, &mDockInstrumentEditor);
    // mDockInstrumentEditor.setFloating(true);
    // mDockInstrumentEditor.hide();

    // addDockWidget(Qt::RightDockWidgetArea, &mDockWaveformEditor);
    // mDockWaveformEditor.setFloating(true);
    // mDockWaveformEditor.hide();

    addDockWidget(Qt::RightDockWidgetArea, &mDockHistory);
    mDockHistory.setFloating(true);
    mDockHistory.hide();
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
    setWindowTitle(QStringLiteral("%1[*] - Trackerboy").arg(mModuleFile.name()));
}

void MainWindow::updateOrderActions() {
    bool canInsert = mOrderModel->canInsert();
    mActionOrderInsert->setEnabled(canInsert);
    mActionOrderDuplicate->setEnabled(canInsert);
    mActionOrderRemove->setEnabled(mOrderModel->canRemove());
    mActionOrderMoveUp->setEnabled(mOrderModel->canMoveUp());
    mActionOrderMoveDown->setEnabled(mOrderModel->canMoveDown());
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

    // if (mMidi.isEnabled()) {
    //     if (QApplication::activeModalWidget()) {
    //         return; // ignore if a dialog is open
    //     }

    //     // MIDI events by default go to the pattern editor
    //     IMidiReceiver *receiver = &mPatternEditor;

    //     QWidget *widget = newWidget;
    //     while (widget) {
    //         // search if this widget's parent is the instrument or waveform editor dock
    //         // if it is, midi events will go to the editor's piano widget
    //         if (widget == &mDockWaveformEditor) {
    //             receiver = &mWaveEditor.piano();
    //             break;
    //         } else if (widget == &mDockInstrumentEditor) {
    //             receiver = &mInstrumentEditor.piano();
    //             break;
    //         }
    //         widget = widget->parentWidget();
    //     }

    //     if (mMidiReceiver != receiver) {
    //         // change the receiver
    //         if (mMidiNoteDown) {
    //             // force the note off
    //             // if we don't do this, the previous receiver won't get the next noteOff message
    //             // and the note will be held indefinitely
    //             mMidiReceiver->midiNoteOff();
    //             mMidiNoteDown = false;
    //         }
    //         mMidiReceiver = receiver;

    //     }
    // }

}
