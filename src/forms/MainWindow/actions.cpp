
#include "forms/MainWindow.hpp"

#include "core/misc/connectutils.hpp"
#include "utils/IconLocator.hpp"

#include <QAction>
#include <QApplication>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>


#define TU MainWindowTU
namespace TU {

//
// adds a new action to a menu and sets the text, status tip and shortcut
//
static QAction* setupAction(QMenu *menu, QString const& text, QString const& statusTip, QKeySequence const &seq = QKeySequence()) {
    auto act = menu->addAction(text);
    act->setStatusTip(statusTip);
    act->setShortcut(seq);
    return act;
}

// same as above, but also sets the icon
static QAction* setupAction(QMenu *menu, QString const& text, QString const& statusTip, Icons icon, QKeySequence const &seq = QKeySequence()) {
    auto act = setupAction(menu, text, statusTip, seq);
    act->setIcon(IconLocator::get(icon));
    return act;
}

static QAction* createAction(QWidget *parent, QString const& text, QString const& statusTip, QKeySequence const &seq = QKeySequence()) {
    auto act = new QAction(text, parent);
    act->setStatusTip(statusTip);
    act->setShortcut(seq);
    return act;
}

static QAction* createAction(QWidget *parent, QString const& text, QString const& statusTip, Icons icon, QKeySequence const &seq = QKeySequence()) {
    auto act = createAction(parent, text, statusTip, seq);
    act->setIcon(IconLocator::get(icon));
    return act;
}

}

void MainWindow::createActions(TableActions const& instrumentActions, TableActions const& waveformActions) {

    auto menubar = menuBar();
    QAction *act;

    // > File ================================================================
    auto menuFile = menubar->addMenu(tr("&File"));

    act = TU::setupAction(menuFile, tr("&New"), tr("Create a new module"), Icons::fileNew, QKeySequence::New);
    mToolbarFile->addAction(act);
    connectActionToThis(act, onFileNew);

    act = TU::setupAction(menuFile, tr("&Open"), tr("Open an existing module"), Icons::fileOpen, QKeySequence::Open);
    mToolbarFile->addAction(act);
    connectActionToThis(act, onFileOpen);
    
    act = TU::setupAction(menuFile, tr("&Save"), tr("Save the module"), Icons::fileSave, QKeySequence::Save);
    mToolbarFile->addAction(act);
    connectActionToThis(act, onFileSave);
    
    act = TU::setupAction(menuFile, tr("Save &As..."), tr("Save the module to a new file"), QKeySequence::SaveAs);
    connectActionToThis(act, onFileSaveAs);

    menuFile->addSeparator(); // ---------------------------------------------
    
    act = TU::setupAction(menuFile, tr("Export to WAV..."), tr("Exports the module to a WAV file"));
    connectActionToThis(act, showExportWavDialog);

    mRecentFilesSeparator = menuFile->addSeparator(); // ---------------------
    mRecentFilesSeparator->setVisible(false);

    // recent files
    for (auto &recent : mRecentFilesActions) {
        recent = new QAction(this);
        recent->setVisible(false);
        menuFile->addAction(recent);
        connectActionToThis(recent, onFileRecent);
    }

    updateRecentFiles();

    menuFile->addSeparator(); // ---------------------------------------------
    
    act = TU::setupAction(menuFile, tr("&Configuration..."), tr("Change application settings"), Icons::fileConfig);
    mToolbarFile->addSeparator();
    mToolbarFile->addAction(act);
    connectActionToThis(act, showConfigDialog);

    menuFile->addSeparator(); // ---------------------------------------------
    
    act = TU::setupAction(menuFile, tr("&Quit"), tr("Exit the application"), QKeySequence::Quit);
    connectActionToThis(act, close);

    // > Edit ================================================================
    auto menuEdit = menubar->addMenu(tr("&Edit"));

    auto undoGroup = mModule->undoGroup();
    act = undoGroup->createUndoAction(this);
    act->setIcon(IconLocator::get(Icons::editUndo));
    act->setShortcut(QKeySequence::Undo);
    mToolbarEdit->addAction(act);
    menuEdit->addAction(act);

    act = undoGroup->createRedoAction(this);
    act->setIcon(IconLocator::get(Icons::editRedo));
    act->setShortcut(QKeySequence::Redo);
    mToolbarEdit->addAction(act);
    menuEdit->addAction(act);

    menuEdit->addSeparator(); // ---------------------------------------------
    mToolbarEdit->addSeparator();

    act = TU::setupAction(menuEdit, tr("C&ut"), tr("Copies and deletes selection to the clipboard"), Icons::editCut, QKeySequence::Cut);
    mToolbarEdit->addAction(act);
    connectActionTo(act, mPatternEditor, cut);

    act = TU::setupAction(menuEdit, tr("&Copy"), tr("Copies selected rows to the clipboard"), Icons::editCopy, QKeySequence::Copy);
    mToolbarEdit->addAction(act);
    connectActionTo(act, mPatternEditor, copy);
    
    act = TU::setupAction(menuEdit, tr("&Paste"), tr("Pastes contents at the cursor"), Icons::editPaste, QKeySequence::Paste);
    mToolbarEdit->addAction(act);
    connectActionTo(act, mPatternEditor, paste);
    
    act = TU::setupAction(menuEdit, tr("Paste &Mix"), tr("Pastes contents at the cursor, merging with existing rows"), tr("Ctrl+M"));
    connectActionTo(act, mPatternEditor, pasteMix);

    act = TU::setupAction(menuEdit, tr("&Erase"), tr("Erases selection contents"), QKeySequence::Delete);
    connectActionTo(act, mPatternEditor, erase);
    
    menuEdit->addSeparator(); // ---------------------------------------------

    act = TU::setupAction(menuEdit, tr("&Select All"), tr("Selects entire track/pattern"), QKeySequence::SelectAll);
    connectActionTo(act, mPatternEditor, selectAll);

    menuEdit->addSeparator(); // ----------------------------------------------

    // > Edit > Transpose
    auto menuEditTranspose = menuEdit->addMenu(tr("Transpose"));
    
    act = TU::setupAction(menuEditTranspose, tr("Decrease note"), tr("Decreases note/notes by 1 step"));
    connectActionTo(act, mPatternEditor, decreaseNote);
    
    act = TU::setupAction(menuEditTranspose, tr("Increase note"), tr("Increases note/notes by 1 step"));
    connectActionTo(act, mPatternEditor, increaseNote);
    
    act = TU::setupAction(menuEditTranspose, tr("Decrease octave"), tr("Decreases note/notes by 12 steps"));
    connectActionTo(act, mPatternEditor, decreaseOctave);
    
    act = TU::setupAction(menuEditTranspose, tr("Increase octave"), tr("Increases note/notes by 12 steps"));
    connectActionTo(act, mPatternEditor, increaseOctave);

    act = TU::setupAction(menuEditTranspose, tr("Custom..."), tr("Transpose by a custom amount"), tr("Ctrl+T"));
    connectActionTo(act, mPatternEditor, transpose);

    TU::setupAction(menuEdit, tr("&Reverse"), tr("Reverses selected rows"), tr("Ctrl+R"));
    connectActionTo(act, mPatternEditor, reverse);

    menuEdit->addSeparator(); // ----------------------------------------------

    act = TU::setupAction(menuEdit, tr("Key repetition"), tr("Toggles key repetition for pattern editor"));
    act->setCheckable(true);
    act->setChecked(true);
    mToolbarInput->addAction(act);
    lazyconnect(act, toggled, mPatternEditor, setKeyRepeat);

    // > Module ===============================================================

    auto menuModule = menubar->addMenu(tr("Module"));

    menuModule->addAction(mSidebar->previousSongAction());

    menuModule->addAction(mSidebar->nextSongAction());

    menuModule->addSeparator(); // --------------------------------------------

    act = TU::setupAction(menuModule, tr("Comments..."), tr("Shows comments about the module"));
    connectActionToThis(act, onModuleComments);

    act = TU::setupAction(menuModule, tr("Module properties..."), tr("Opens the module properties dialog"));
    connectActionToThis(act, onModuleModuleProperties);

    // > Song =================================================================

    mActionOrderInsert = TU::createAction(this, tr("&Insert order row"), tr("Inserts a new order at the current pattern"), Icons::itemAdd);
    connectActionTo(mActionOrderInsert, mPatternModel, insertOrder);
    
    mActionOrderRemove = TU::createAction(this, tr("&Remove order row"), tr("Removes the order at the current pattern"), Icons::itemRemove);
    connectActionTo(mActionOrderRemove, mPatternModel, removeOrder);

    mActionOrderDuplicate = TU::createAction(this, tr("&Duplicate order row"), tr("Duplicates the order at the current pattern"), Icons::itemDuplicate);
    connectActionTo(mActionOrderDuplicate, mPatternModel, duplicateOrder);

    mActionOrderMoveUp = TU::createAction(this, tr("Move order &up"), tr("Moves the order up 1"), Icons::moveUp);
    connectActionTo(mActionOrderMoveUp, mPatternModel, moveOrderUp);

    mActionOrderMoveDown = TU::createAction(this, tr("Move order dow&n"), tr("Moves the order down 1"), Icons::moveDown);
    connectActionTo(mActionOrderMoveDown, mPatternModel, moveOrderDown);

    auto menuSong = menubar->addMenu(tr("&Song"));
    setupSongMenu(menuSong);

    menuSong->addSeparator();

    act = TU::setupAction(menuSong, tr("Tempo calculator..."), tr("Shows the tempo calculator dialog"));
    connectActionToThis(act, showTempoCalculator);

    // > Instrument ===========================================================
    auto menuInstrument = menubar->addMenu(tr("Instrument"));

    setupTableMenu(menuInstrument, instrumentActions);

    // > Waveform =============================================================
    auto menuWaveform = menubar->addMenu(tr("Waveform"));

    setupTableMenu(menuWaveform, waveformActions);

    // > Tracker ==============================================================
    auto menuTracker = menubar->addMenu(tr("&Tracker"));

    act = TU::setupAction(menuTracker, tr("&Play"), tr("Resume playing or play the song from the current position"), Icons::trackerPlay);
    mToolbarTracker->addAction(act);
    connectActionToThis(act, onTrackerPlay);

    act = TU::setupAction(menuTracker, tr("Play from start"), tr("Begin playback of the song from the start"), Icons::trackerRestart, QKeySequence(Qt::Key_F5));
    mToolbarTracker->addAction(act);
    connectActionToThis(act, onTrackerPlayAtStart);

    act = TU::setupAction(menuTracker, tr("Play at cursor"), tr("Begin playback from the cursor"), Icons::trackerPlayRow, QKeySequence(Qt::Key_F6));
    mToolbarTracker->addAction(act);
    connectActionToThis(act, onTrackerPlayFromCursor);

    act = TU::setupAction(menuTracker, tr("Step row"), tr("Play and hold the row at the cursor"), Icons::trackerStepRow, QKeySequence(Qt::Key_F7));
    mToolbarTracker->addAction(act);
    connectActionToThis(act, onTrackerStep);

    act = TU::setupAction(menuTracker, tr("&Stop"), tr("Stop playing"), Icons::trackerStop, QKeySequence(Qt::Key_F8));
    mToolbarTracker->addAction(act);
    connectActionToThis(act, onTrackerStop);

    mToolbarTracker->addSeparator();

    act = TU::setupAction(menuTracker, "Pattern repeat", "Toggles pattern repeat mode", Icons::trackerRepeat, QKeySequence(Qt::Key_F9));
    act->setCheckable(true);
    mToolbarTracker->addAction(act);
    lazyconnect(act, toggled, mRenderer, setPatternRepeat);

    act = TU::setupAction(menuTracker, tr("Record"), tr("Toggles record mode"), Icons::trackerRecord, QKeySequence(Qt::Key_Space));
    act->setCheckable(true);
    mToolbarTracker->addAction(act);
    lazyconnect(act, toggled, mPatternModel, setRecord);

    mToolbarTracker->addAction(mSidebar->previousSongAction());
    mToolbarTracker->addAction(mSidebar->nextSongAction());

    menuTracker->addSeparator(); // -------------------------------------------

    act = TU::setupAction(menuTracker, tr("Toggle channel output"), tr("Enables/disables sound output for the current track"), QKeySequence(Qt::Key_F10));
    connectActionToThis(act, onTrackerToggleOutput);

    act = TU::setupAction(menuTracker, tr("Solo"), tr("Solos the current track"), QKeySequence(Qt::Key_F11));
    connectActionToThis(act, onTrackerSolo);

    menuTracker->addSeparator(); // -------------------------------------------

    act = TU::setupAction(menuTracker, tr("&Kill sound"), tr("Immediately stops sound output"), QKeySequence(Qt::Key_F12));
    connectActionToThis(act, onTrackerKill);

    mActionFollowMode = TU::createAction(this, tr("Follow-mode"), tr("Toggles follow mode"), Qt::Key_ScrollLock);
    mActionFollowMode->setCheckable(true);
    mActionFollowMode->setChecked(true);
    mToolbarTracker->addAction(mActionFollowMode);
    lazyconnect(mActionFollowMode, toggled, mPatternModel, setFollowing);

    // > View =================================================================
    auto menuView = menubar->addMenu(tr("&View"));

    mActionViewReset = TU::createAction(this, tr("Reset layout"), tr("Rearranges all docks and toolbars to the default layout"));
    connectActionToThis(mActionViewReset, onViewResetLayout);

    setupViewMenu(menuView);

    // > Help =================================================================
    auto menuHelp = menubar->addMenu(tr("&Help"));

    #ifdef QT_DEBUG
    // secret debug menu

    auto menuHelpDebug = menuHelp->addMenu(tr("Debug"));
    act = TU::setupAction(menuHelpDebug, tr("Dump QObject tree"), tr("Calls MainWindow::dumpObjectTree()"));
    connect(act, &QAction::triggered, this, [this]() {
        dumpObjectTree();
    });

    act = TU::setupAction(menuHelpDebug, tr("Clear config"), tr("Deletes the application configuration, window state and geometry"));
    connect(act, &QAction::triggered, this, [this](){
        mSaveConfig = false;
        QSettings settings;
        settings.clear();
    });

    act = TU::setupAction(menuHelpDebug, tr("Panic"), tr("Forces a fatal error to occur"));
    connect(act, &QAction::triggered, this, []() {
        qFatal("panic requested");
    });

    #endif

    act = TU::setupAction(menuHelp, tr("Audio diagnostics..."), tr("Shows the audio diagnostics dialog"));
    connectActionToThis(act, showAudioDiag);
    
    menuHelp->addSeparator(); // ----------------------------------------------
    
    act = TU::setupAction(menuHelp, tr("&About"), tr("About this program"));
    connectActionToThis(act, showAboutDialog);
    
    act = TU::setupAction(menuHelp, tr("About &Qt"), tr("Shows information about Qt"));
    QApplication::connect(act, &QAction::triggered, &QApplication::aboutQt);


}

void MainWindow::setupSongMenu(QMenu *menu) {
    menu->addAction(mActionOrderInsert);
    menu->addAction(mActionOrderRemove);
    menu->addAction(mActionOrderDuplicate);
    menu->addSeparator();
    menu->addAction(mActionOrderMoveUp);
    menu->addAction(mActionOrderMoveDown); 
}

void MainWindow::setupViewMenu(QMenu *menu) {
    menu->addAction(mDockInstruments->toggleViewAction());
    menu->addAction(mDockWaveforms->toggleViewAction());
    menu->addAction(mDockHistory->toggleViewAction());
    menu->addSeparator();
    auto toolbarMenu = menu->addMenu(tr("Toolbars"));
    for (auto toolbar : {
            mToolbarFile,
            mToolbarEdit, 
            mToolbarSong,
            mToolbarTracker,
            mToolbarInput
        }) {
        toolbarMenu->addAction(toolbar->toggleViewAction());
    }
    menu->addSeparator();   
    menu->addAction(mActionViewReset);
}

void MainWindow::setupTableMenu(QMenu *menu, TableActions const& actions) {

    menu->addAction(actions.add);
    menu->addAction(actions.remove);
    menu->addAction(actions.duplicate);

    menu->addSeparator();
    
    menu->addAction(actions.importFile);
    menu->addAction(actions.exportFile);

    menu->addSeparator();
    
    menu->addAction(actions.edit);

}

#undef TU
