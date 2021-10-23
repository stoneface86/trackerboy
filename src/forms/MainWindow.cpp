
#include "MainWindow.hpp"

#include "core/misc/connectutils.hpp"
#include "core/IconManager.hpp"
#include "widgets/docks/TableDock.hpp"
#include "version.hpp"

#include <QApplication>
#include <QFileInfo>
#include <QSettings>
#include <QShortcut>
#include <QScreen>
#include <QMenu>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QtDebug>
#include <QUndoView>

#define TU MainWindowTU

namespace TU {

static auto const KEY_MAIN_WINDOW = QStringLiteral("MainWindow");
static auto const KEY_WINDOW_STATE = QStringLiteral("windowState");
static auto const KEY_GEOMETRY = QStringLiteral("geometry");

static auto const KEY_RECENT_FILES = QStringLiteral("recentFiles");

//
// increment this constant when adding new docks or toolbars
//
static constexpr int WINDOW_STATE_VERSION = 1;

}

MainWindow::MainWindow() :
    QMainWindow(),
    mUntitledString(tr("Untitled")),
    mConfig(),
    mMidi(),
    mMidiReceiver(nullptr),
    mMidiNoteDown(false),
    mModule(),
    mModuleFile(),
    mErrorSinceLastConfig(false),
    mAboutDialog(nullptr),
    mAudioDiag(nullptr),
    mConfigDialog(nullptr),
    mTempoCalc(nullptr),
    mCommentsDialog(nullptr),
    mInstrumentEditor(nullptr),
    mWaveEditor(nullptr)
{

    // create models
    mModule = new Module(this);
    mInstrumentModel = new InstrumentListModel(*mModule, this);
    mSongListModel = new SongListModel(*mModule, this);
    mSongModel = new SongModel(*mModule, this);
    mPatternModel = new PatternModel(*mModule, *mSongModel, this);
    mWaveModel = new WaveListModel(*mModule, this);

    mRenderer = new Renderer(*mModule, this);

    setupUi();

    // read in application configuration
    mConfig.readSettings();
    
    setWindowIcon(IconManager::getAppIcon());

    QSettings settings;
    settings.beginGroup(TU::KEY_MAIN_WINDOW);

    // restore geomtry from the last session
    auto const geometry = settings.value(TU::KEY_GEOMETRY, QByteArray()).toByteArray();

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
    auto const windowState = settings.value(TU::KEY_WINDOW_STATE).toByteArray();
    if (windowState.isEmpty()) {
        // default layout
        initState();
    } else {
        addToolBar(mToolbarFile);
        addToolBar(mToolbarEdit);
        addToolBar(mToolbarSong);
        addToolBar(mToolbarTracker);
        addToolBar(mToolbarInput);
        addDockWidget(Qt::LeftDockWidgetArea, mDockHistory);
        addDockWidget(Qt::LeftDockWidgetArea, mDockInstruments);
        addDockWidget(Qt::LeftDockWidgetArea, mDockWaveforms);
        if (!restoreState(windowState, TU::WINDOW_STATE_VERSION)) {
            initState();
        }
    }

    mModuleFile.setName(mUntitledString);
    updateWindowTitle();

    // apply the read in configuration
    onConfigApplied(Config::CategoryAll);

    setStyleSheet(QStringLiteral(R"stylesheet(
QToolBar QLabel {
    padding-left: 3px;
    padding-right: 3px;
}
    )stylesheet"));
}

MainWindow::~MainWindow() {

}

QMenu* MainWindow::createPopupMenu() {
    auto menu = new QMenu(this);
    setupViewMenu(menu);
    return menu;
}

void MainWindow::panic(QString const& msg) {
    // this function is called from our custom message handler for fatal
    // messages. Shows the fatal error to the user and attempts to save the
    // current module if it was modified.

    QString moduleSaveResult;
    if (mModule->isModified()) {
        auto path = mModuleFile.crashSave(*mModule);
        if (path.isEmpty()) {
            moduleSaveResult = tr("Unable to save a copy of the module");
        } else {
            moduleSaveResult = tr("A copy of the module has been saved to:\n%1").arg(path);
        }
    }

    QMessageBox msgbox(this);
    msgbox.setIcon(QMessageBox::Critical);
    msgbox.setText(tr("An unexpected error has occurred:"));
    if (moduleSaveResult.isEmpty()) {
        msgbox.setInformativeText(msg);
    } else {
        msgbox.setInformativeText(tr("%1\n\n%2").arg(msg, moduleSaveResult));
    }
    msgbox.exec();
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    if (maybeSave()) {
        // user saved or discarded changes, close the window
        #ifdef QT_DEBUG
        if (mSaveConfig) {
        #endif
            QSettings settings;
            settings.beginGroup(TU::KEY_MAIN_WINDOW);
            settings.setValue(TU::KEY_GEOMETRY, saveGeometry());
            settings.setValue(TU::KEY_WINDOW_STATE, saveState(TU::WINDOW_STATE_VERSION));
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
    mSidebar = new Sidebar(*mModule, *mPatternModel, *mSongListModel, *mSongModel);
    mPatternEditor = new PatternEditor(mConfig.pianoInput(), *mPatternModel);
    layout->addWidget(mSidebar);
    layout->addWidget(mPatternEditor, 1);
    centralWidget->setLayout(layout);

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

    mDockInstruments = makeDock(tr("Instruments"), QStringLiteral("DockInstruments"));
    auto instruments = new TableDock(*mInstrumentModel, tr("Ctrl+I"), tr("instrument"), mDockInstruments);
    connect(instruments, &TableDock::selectedItemChanged, this,
        [this](int index) {
            int id = -1;
            if (index != -1) {
                id = mInstrumentModel->id(index);
            }
            mPatternEditor->setInstrument(id);
        });
    mDockInstruments->setWidget(instruments);

    mDockWaveforms = makeDock(tr("Waveforms"), QStringLiteral("DockWaveforms"));
    auto waveforms = new TableDock(*mWaveModel, tr("Ctrl+W"), tr("waveform"), mDockWaveforms);
    mDockWaveforms->setWidget(waveforms);

    // TOOLBARS ==============================================================

    mToolbarFile = makeToolbar(tr("File"), QStringLiteral("ToolbarFile"));
    mToolbarEdit = makeToolbar(tr("Edit"), QStringLiteral("ToolbarEdit"));
    mToolbarSong = makeToolbar(tr("Song"), QStringLiteral("ToolbarSong"));
    mToolbarTracker = makeToolbar(tr("Tracker"), QStringLiteral("ToolbarTracker"));
    mToolbarInput = makeToolbar(tr("Input"), QStringLiteral("ToolbarInput"));

    mOctaveSpin = new QSpinBox(mToolbarInput);
    mOctaveSpin->setRange(2, 8);
    mOctaveSpin->setValue(mConfig.pianoInput().octave());
    connect(mOctaveSpin, qOverload<int>(&QSpinBox::valueChanged), this,
        [this](int octave) {
            mConfig.pianoInput().setOctave(octave);
        });
    auto editStepSpin = new QSpinBox(mToolbarInput);
    editStepSpin->setRange(0, 255);
    editStepSpin->setValue(1);
    connect(editStepSpin, qOverload<int>(&QSpinBox::valueChanged), mPatternEditor, &PatternEditor::setEditStep);
    mToolbarInput->addWidget(new QLabel(tr("Octave"), mToolbarInput));
    mToolbarInput->addWidget(mOctaveSpin);
    mToolbarInput->addWidget(new QLabel(tr("Edit step"), mToolbarInput));
    mToolbarInput->addWidget(editStepSpin);
    mToolbarInput->addSeparator();

    // ACTIONS ===============================================================

    createActions(instruments->tableActions(), waveforms->tableActions());

    mToolbarSong->addAction(mActionOrderInsert);
    mToolbarSong->addAction(mActionOrderRemove);
    mToolbarSong->addAction(mActionOrderDuplicate);
    mToolbarSong->addAction(mActionOrderMoveUp);
    mToolbarSong->addAction(mActionOrderMoveDown);

    // SHORTCUTS =============================================================

    QShortcut *shortcut;

    shortcut = new QShortcut(tr("Ctrl+Left"), this);
    lazyconnect(shortcut, activated, this, previousInstrument);

    shortcut = new QShortcut(tr("Ctrl+Right"), this);
    lazyconnect(shortcut, activated, this, nextInstrument);

    shortcut = new QShortcut(tr("Ctrl+Up"), this);
    lazyconnect(shortcut, activated, this, previousPattern);
    
    shortcut = new QShortcut(tr("Ctrl+Down"), this);
    lazyconnect(shortcut, activated, this, nextPattern);

    shortcut = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_BracketRight), this);
    lazyconnect(shortcut, activated, this, increaseOctave);
    
    shortcut = new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_BracketLeft), this);
    lazyconnect(shortcut, activated, this, decreaseOctave);

    shortcut = new QShortcut(QKeySequence(Qt::Key_Return), mPatternEditor);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    lazyconnect(shortcut, activated, this, playOrStop);

    // STATUSBAR ==============================================================

    auto statusbar = statusBar();

    mStatusRenderer = new QLabel(statusbar);
    mStatusSpeed = new SpeedLabel(statusbar);
    mStatusTempo = new TempoLabel(statusbar);
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
            (QLabel*)mStatusSpeed,
            (QLabel*)mStatusTempo,
            mStatusElapsed,
            mStatusPos,
            mStatusSamplerate
            }) {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        statusbar->addPermanentWidget(label);
    }

    statusbar->showMessage(tr("Trackerboy v%1").arg(QString::fromLatin1(VERSION_STR)));

    // default statuses
    setPlayingStatus(PlayingStatusText::ready);
    mStatusElapsed->setText(QStringLiteral("00:00"));
    mStatusPos->setText(QStringLiteral("00 / 00"));
    // no need to set samplerate, it is done so in onConfigApplied

    // CONNECTIONS ============================================================

    auto orderEditor = mSidebar->orderEditor();
    connect(orderEditor, &OrderEditor::popupMenuAt, this,
        [this](QPoint const& pos) {
            if (mSongOrderContextMenu == nullptr) {
                mSongOrderContextMenu = new QMenu(this);
                setupSongMenu(mSongOrderContextMenu);
            }
            mSongOrderContextMenu->popup(pos);
        });
    auto orderGrid = orderEditor->grid();
    lazyconnect(orderGrid, patternJump, mRenderer, jumpToPattern);
    orderGrid->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(orderGrid, &OrderGrid::customContextMenuRequested, this,
        [this](QPoint const& pos) {
            auto _sender = qobject_cast<QWidget*>(sender());
            if (_sender) {
                QMenu *menu = new QMenu(this);
                menu->setAttribute(Qt::WA_DeleteOnClose);
                menu->addAction(mActionOrderInsert);
                menu->addAction(mActionOrderRemove);
                menu->addAction(mActionOrderDuplicate);
                menu->addSeparator();
                menu->addAction(mActionOrderMoveUp);
                menu->addAction(mActionOrderMoveDown);
                menu->popup(_sender->mapToGlobal(pos));
            }
        });

    lazyconnect(mPatternEditor, previewNote, mRenderer, instrumentPreview);
    lazyconnect(mPatternEditor, stopNotePreview, mRenderer, stopPreview);

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

    connect(mRenderer, &Renderer::audioStarted, this, &MainWindow::onAudioStart);
    connect(mRenderer, &Renderer::audioStopped, this, &MainWindow::onAudioStop);
    connect(mRenderer, &Renderer::audioError, this, &MainWindow::onAudioError);
    connect(mRenderer, &Renderer::frameSync, this, &MainWindow::onFrameSync);
    
    auto scope = mSidebar->scope();
    scope->setBuffer(&mRenderer->visualizerBuffer());
    connect(mRenderer, &Renderer::updateVisualizers, scope, qOverload<>(&AudioScope::update));

    lazyconnect(mRenderer, isPlayingChanged, mPatternModel, setPlaying);

    lazyconnect(instruments, edit, this, editInstrument);
    lazyconnect(waveforms, edit, this, editWaveform);

    connect(mPatternEditor->gridHeader(), &PatternGridHeader::outputChanged, mRenderer, &Renderer::setChannelOutput);

    lazyconnect(mPatternModel, patternCountChanged, this, onPatternCountChanged);
    lazyconnect(mPatternModel, cursorPatternChanged, this, onPatternCursorChanged);

    auto app = static_cast<QApplication*>(QApplication::instance());
    connect(app, &QApplication::focusChanged, this, &MainWindow::handleFocusChange);

    onPatternCountChanged(mPatternModel->patterns());
    onPatternCursorChanged(mPatternModel->cursorPattern());
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

    addDockWidget(Qt::RightDockWidgetArea, mDockHistory);
    mDockHistory->setFloating(true);
    mDockHistory->hide();

    addDockWidget(Qt::RightDockWidgetArea, mDockInstruments);
    mDockInstruments->setFloating(true);
    mDockInstruments->hide();

    addDockWidget(Qt::RightDockWidgetArea, mDockWaveforms);
    mDockWaveforms->setFloating(true);
    mDockWaveforms->hide();
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
        IMidiReceiver *receiver = mPatternEditor;

        QWidget *widget = newWidget;
        while (widget) {
            // search if this widget's parent is the instrument or waveform editor dock
            // if it is, midi events will go to the editor's piano widget
            if (widget == mWaveEditor) {
                receiver = mWaveEditor->piano();
                break;
            } else if (widget == mInstrumentEditor) {
                receiver = mInstrumentEditor->piano();
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

namespace TU {

//
// Makes the size of the given list no larger than max
//
static void minimizeList(QStringList &list, int max) {
    while (list.size() > max) {
        list.takeLast();
    }
}

}

void MainWindow::pushRecentFile(const QString &file) {
    QSettings settings;
    settings.beginGroup(TU::KEY_MAIN_WINDOW);

    auto list = settings.value(TU::KEY_RECENT_FILES).toStringList();
    // remove duplicate(s)
    list.removeAll(file);
    list.prepend(file);
    TU::minimizeList(list, (int)mRecentFilesActions.size());

    settings.setValue(TU::KEY_RECENT_FILES, list);
    updateRecentFiles(list);

}

void MainWindow::updateRecentFiles() {
    QSettings settings;
    settings.beginGroup(TU::KEY_MAIN_WINDOW);
    auto list = settings.value(TU::KEY_RECENT_FILES).toStringList();
    TU::minimizeList(list, (int)mRecentFilesActions.size());
    updateRecentFiles(list);
}

void MainWindow::updateRecentFiles(const QStringList &list) {

    int const size = list.size();
    mRecentFilesSeparator->setVisible(size > 0);

    int i = 0;
    for (auto const& filename : list) {
        QFileInfo info(filename);
        QString text = tr("&%1 %2").arg(QString::number(i + 1), info.fileName());
        auto act = mRecentFilesActions[i];
        act->setText(text);
        act->setStatusTip(filename); // put the full filename in the statusbar in case of duplicates
        act->setVisible(true);

        ++i;
    }


    for (auto iter = mRecentFilesActions.begin() + size; iter != mRecentFilesActions.end(); ++iter) {
        (*iter)->setVisible(false);
    }
}

#undef TU
