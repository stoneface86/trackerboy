
#include "MainWindow.hpp"

#include "core/model/InstrumentChoiceModel.hpp"
#include "misc/IconManager.hpp"
#include "misc/connectutils.hpp"

#include <QApplication>
#include <QSettings>
#include <QShortcut>
#include <QScreen>
#include <QMenu>
#include <QStatusBar>
#include <QtDebug>
#include <QUndoView>

static auto const KEY_WINDOW_STATE = QStringLiteral("windowState");
static auto const KEY_GEOMETRY = QStringLiteral("geometry");

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
    mConfigDialog(nullptr)
{

    // create models
    mModule = new Module(this);
    mInstrumentModel = new InstrumentListModel(*mModule, this);
    mSongModel = new SongModel(*mModule, this);
    mPatternModel = new PatternModel(*mModule, *mSongModel, this);
    mWaveModel = new WaveListModel(*mModule, this);

    setupUi();

    // read in application configuration
    mConfig.readSettings();
    
    setWindowIcon(IconManager::getAppIcon());

    QSettings settings;

    // restore geomtry from the last session
    auto const geometry = settings.value(KEY_GEOMETRY, QByteArray()).toByteArray();

    if (geometry.isEmpty()) {
        // no saved geometry, initialize it
        // we will take 3/4 of the primary screen's width and height, but we
        // will take no more than 1280x720

        // maximum initial resolution
        constexpr int MAX_WIDTH = 1280;
        constexpr int MAX_HEIGHT = 720;

        // get the available geometry for the primary screen
        auto const availableGeometry = QApplication::primaryScreen()->availableGeometry();

        QRect newGeometry(
            0,
            0, 
            std::min(MAX_WIDTH, availableGeometry.width() * 3 / 4),
            std::min(MAX_HEIGHT, availableGeometry.height() * 3 / 4)
        );

        newGeometry.moveTo(availableGeometry.center() - newGeometry.center());
        setGeometry(newGeometry);
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
        // addDockWidget(Qt::LeftDockWidgetArea, mDockInstrumentEditor);
        // addDockWidget(Qt::LeftDockWidgetArea, mDockWaveformEditor);
        addDockWidget(Qt::LeftDockWidgetArea, mDockHistory);
        restoreState(windowState);
    }

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

void MainWindow::showEvent(QShowEvent *evt) {
    Q_UNUSED(evt)
    mPatternEditor->setFocus();
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

void MainWindow::commitModels() {
    // libtrackerboy uses std::string, models use QString
    // models use a QString version of the underlying std::string data
    // commiting converts the QString data (if needed) to std::string
    mInstrumentModel->commit();
    mWaveModel->commit();
}

QDockWidget* MainWindow::makeDock(QString const& title, QString const& objname) {
    auto dock = new QDockWidget(title, this);
    dock->setObjectName(objname);
    return dock;
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
    mPatternEditor = new PatternEditor(mPianoInput);
    mPatternEditor->setModel(mPatternModel);
    layout->addWidget(mSidebar);
    layout->addWidget(mPatternEditor, 1);
    centralWidget->setLayout(layout);

    //mSidebar->orderEditor()->setModel(mOrderModel);
    mSidebar->songEditor()->setModel(mSongModel);

    setCentralWidget(centralWidget);

    {
        auto grid = mPatternEditor->grid();
        grid->setFirstHighlight(mSongModel->rowsPerBeat());
        grid->setSecondHighlight(mSongModel->rowsPerMeasure());
        lazyconnect(mSongModel, rowsPerBeatChanged, grid, setFirstHighlight);
        lazyconnect(mSongModel, rowsPerMeasureChanged, grid, setSecondHighlight);
    }

    // DOCKS =================================================================

    mDockHistory = makeDock(tr("History"), QStringLiteral("DockHistory"));
    auto undoView = new QUndoView(mModule->undoGroup(), mDockHistory);
    mDockHistory->setWidget(undoView);

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

    QShortcut *shortcut;

    shortcut = new QShortcut(tr("Ctrl+Left"), this);
    lazyconnect(shortcut, activated, this, previousInstrument);

    shortcut = new QShortcut(tr("Ctrl+Right"), this);
    lazyconnect(shortcut, activated, this, nextInstrument);

    shortcut = new QShortcut(tr("Ctrl+Up"), this);
    lazyconnect(shortcut, activated, this, previousPattern);
    
    shortcut = new QShortcut(tr("Ctrl+Down"), this);
    lazyconnect(shortcut, activated, this, nextPattern);

    shortcut = new QShortcut(QKeySequence(Qt::KeypadModifier | Qt::Key_Asterisk), this);
    lazyconnect(shortcut, activated, this, increaseOctave);
    
    shortcut = new QShortcut(QKeySequence(Qt::KeypadModifier | Qt::Key_Slash), this);
    lazyconnect(shortcut, activated, this, decreaseOctave);

    // STATUSBAR ==============================================================

    auto statusbar = statusBar();

    mStatusRenderer = new QLabel(statusbar);
    mStatusSpeed = new QLabel(statusbar);
    mStatusTempo = new QLabel(statusbar);
    mStatusElapsed = new QLabel(statusbar);
    mStatusPos = new QLabel(statusbar);
    mStatusSamplerate = new QLabel(statusbar);

    mStatusRenderer->setMinimumWidth(60);
    mStatusSpeed->setMinimumWidth(60);
    mStatusTempo->setMinimumWidth(60);
    mStatusElapsed->setMinimumWidth(40);
    mStatusPos->setMinimumWidth(40);
    mStatusSamplerate->setMinimumWidth(60);

    for (auto label : { 
            mStatusRenderer,
            mStatusSpeed,
            mStatusTempo,
            mStatusElapsed,
            mStatusPos,
            mStatusSamplerate
            }) {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        statusbar->addPermanentWidget(label);
    }

    statusbar->showMessage(tr("Trackerboy v%1.%2.%3")
        .arg(trackerboy::VERSION.major)
        .arg(trackerboy::VERSION.minor)
        .arg(trackerboy::VERSION.patch));

    // default statuses
    setPlayingStatus(PlayingStatusText::ready);
    mStatusSpeed->setText(tr("6.000 FPR"));
    mStatusTempo->setText(tr("150 BPM"));
    mStatusElapsed->setText(QStringLiteral("00:00"));
    mStatusPos->setText(QStringLiteral("00 / 00"));
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

    //connect(mOrderModel, &OrderModel::currentPatternChanged, this, &MainWindow::updateOrderActions);
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

    // addDockWidget(Qt::RightDockWidgetArea, mDockInstrumentEditor);
    // mDockInstrumentEditor->setFloating(true);
    // mDockInstrumentEditor->hide();

    // addDockWidget(Qt::RightDockWidgetArea, mDockWaveformEditor);
    // mDockWaveformEditor->setFloating(true);
    // mDockWaveformEditor->hide();

    addDockWidget(Qt::RightDockWidgetArea, mDockHistory);
    mDockHistory->setFloating(true);
    mDockHistory->hide();
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
    // bool canInsert = mOrderModel->canInsert();
    // mActionOrderInsert->setEnabled(canInsert);
    // mActionOrderDuplicate->setEnabled(canInsert);
    // mActionOrderRemove->setEnabled(mOrderModel->canRemove());
    // mActionOrderMoveUp->setEnabled(mOrderModel->canMoveUp());
    // mActionOrderMoveDown->setEnabled(mOrderModel->canMoveDown());
}

void MainWindow::setPlayingStatus(PlayingStatusText type) {
    static const char *PLAYING_STATUSES[] = {
        QT_TR_NOOP("Ready"),
        QT_TR_NOOP("Playing"),
        QT_TR_NOOP("Device error")
    };

    mStatusRenderer->setText(tr(PLAYING_STATUSES[(int)type]));
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
    Q_UNUSED(newWidget)

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
