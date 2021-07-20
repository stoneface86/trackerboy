
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


static auto const KEY_WINDOW_STATE = QStringLiteral("windowState");
static auto const KEY_GEOMETRY = QStringLiteral("geometry");


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
    auto const geometry = settings.value(KEY_GEOMETRY, QByteArray()).toByteArray();

    if (geometry.isEmpty()) {
        #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        // screen() was added at version 5.14, making QDesktopWidget obsolete
        auto const availableGeometry = window()->screen()->availableGeometry();
        #else
        auto const availableGeometry = QApplication::desktop()->availableGeometry();
        #endif

        // initialize window size to 3/4 of the screen's width and height
        resize(availableGeometry.width() / 4 * 3, availableGeometry.height() / 4 * 3);
        move((availableGeometry.width() - width()) / 2,
            (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }

    // restore window state if it exists
    auto const windowState = settings.value(KEY_WINDOW_STATE).toByteArray();
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
            settings.setValue(KEY_GEOMETRY, saveGeometry());
            settings.setValue(KEY_WINDOW_STATE, saveState());
        #ifdef QT_DEBUG
        }
        #endif
        evt->accept();
    } else {
        // user aborted closing, ignore this event
        evt->ignore();
    }
    
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
