
#include "forms/MainWindow.hxx"

#include "core/icons.hxx"
#include "core/settings.hxx"
#include "utils/ToolbarLayout.hxx"
#include "utils/actions.hxx"
#include "utils/aliases.hxx"
#include "utils/connectutils.hxx"

#include <QApplication>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QScreen>
#include <QSettings>
#include <QStatusBar>

#define TU MainWindowTU
namespace TU {

static strlit cGroup = "MainWindow";
static strlit cKeyShowSidebar = "showSidebar";
static strlit cKeyShowDatabar = "showDatabar";
static strlit cKeyShowStatusbar = "showStatusBar";
static strlit cKeyOctave = "octave";
static strlit cKeyEditStep = "editStep";
static strlit cKeyKeyRepeat = "keyRepetition";
static strlit cKeyFollowMode = "followMode";
static strlit cKeyPatternRepeat = "patternRepeat";
static strlit cKeyRecord = "record";
static strlit cKeySplitterV = "splitterV";
static strlit cKeySplitterH = "splitterH";

//
// increment this constant when adding new docks or toolbars
// v3 - Removed Song toolbar, added View toolbar
// v2 - removed all dock widgets (no longer using QDockWidget)
// v1 - initial version
//
static const int cWindowStateVers = 3;

} // namespace TU

MainWindow::MainWindow()
    : QMainWindow()
    , mRecentFiles()
    , mDocument(new Document(this))
    , mSongListModel(new SongListModel(mDocument, this))
    , mSongModel(new SongModel(mDocument, mSongListModel, this))
    , mSongListEditor(nullptr)
    , mModuleProperties{}
    , mComments{} {

    lazyconnect(mDocument, modifiedChanged, this, setWindowModified);

    initToolBars();
    initUi();
    initMenuBar();
    initStatusBar();

    loadSettings();

    // TODO: remove this when configuration is done
    ColorTheme theme;
    icons::generate(theme);
    updateIcons();
}

void MainWindow::openFile(QString const &path) {
    Q_UNUSED(path)
}

void MainWindow::panic(QString const &msg) {
    Q_UNUSED(msg)
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    saveSettings();
    evt->accept();
}

void MainWindow::showSongListEditor() {
    if (mSongListEditor == nullptr) {
        mSongListEditor = new SongListEditor(mSongListModel, this);
        connectLambda(mSongListEditor, finished, this, [this](int result) {
            if (result == QDialog::Accepted) {
                mSongListEditor->applyChanges(*mDocument);
            } else {
                mSongListEditor->revertChanges();
            }
        });
    }
    // TODO: Stop playback
    mSongListEditor->open();
}

void MainWindow::showComments() {
    if (mComments == nullptr) {
        mComments = new CommentsDialog(mDocument, this);
    }
    mComments->show();
}

void MainWindow::showModuleProperties() {
    if (mModuleProperties == nullptr) {
        mModuleProperties = new ModulePropertiesDialog(this);
        connectLambda(mModuleProperties, accepted, this, [this]() {
            mModuleProperties->save(*mDocument);
        });
    }
    mModuleProperties->load(*mDocument);
    mModuleProperties->open();
}

void MainWindow::onNew() {}
void MainWindow::onOpen() {}
void MainWindow::onSave() {}
void MainWindow::onSaveAs() {}
void MainWindow::onExportToWav() {}
void MainWindow::onConfiguration() {}

void MainWindow::updateIcons() {
    iterateActions(menuBar(), [](QAction *act) {
        auto const data = getData(act);
        if (data.icon != -1) {
            act->setIcon(icons::get((icons::Icons)data.icon));
        }
    });
}

void MainWindow::initToolBars() {
    auto initToolBar = [this](Toolbars toolbar, QString const &title) {
        auto bar = icons::largeToolBar(this);
        bar->setWindowTitle(title);
        bar->setObjectName(title);
        addToolBar(bar);
        mToolbars[toolbar] = bar;
    };

    initToolBar(ToolbarFile, tr("File"));
    initToolBar(ToolbarEdit, tr("Edit"));
    initToolBar(ToolbarTracker, tr("Tracker"));
    initToolBar(ToolbarInput, tr("Input"));
    initToolBar(ToolbarView, tr("View"));
    {
        auto *bar = mToolbars[ToolbarInput];

        auto container = new QWidget;
        auto layout = new ToolbarLayout(QBoxLayout::LeftToRight);
        layout->addWidget(new QLabel(tr("Octave")));
        mUi.inputOctaveSpin = new QSpinBox;
        mUi.inputOctaveSpin->setValue(5);
        mUi.inputOctaveSpin->setRange(2, 8);
        layout->addWidget(mUi.inputOctaveSpin);
        layout->addWidget(new QLabel(tr("Edit Step")));
        mUi.inputEditStep = new QSpinBox;
        mUi.inputEditStep->setRange(1, 255);
        layout->addWidget(mUi.inputEditStep);
        container->setLayout(layout);
        bar->addWidget(container);
        layout->setToolBar(bar);
    }
}

void MainWindow::initMenuBar() {

    auto const menubar = menuBar();
    QMenu *menu{};

#define A(...) buildAction(menu, __VA_ARGS__)
#define SEP() menu->addSeparator()
#define MENU(text) menu = menubar->addMenu(text)
#define SUBMENU(text)                                                          \
    const auto _menu = menu->addMenu(text);                                    \
    auto menu = _menu

    // =================================================================== File
    MENU(tr("&File"));
    A(tr("&New"), tr("Create a new module"))
        .icon(icons::New)
        .shortcut(QKeySequence::New)
        .triggers(lazyslotx(this, onNew))
        .addTo(mToolbars[ToolbarFile]);
    A(tr("&Open"), tr("Opens a module from a file"))
        .icon(icons::Open)
        .shortcut(QKeySequence::Open)
        .triggers(lazyslotx(this, onOpen))
        .addTo(mToolbars[ToolbarFile]);
    A(tr("&Save"), tr("Saves the module"))
        .icon(icons::Save)
        .shortcut(QKeySequence::Save)
        .triggers(lazyslotx(this, onSave))
        .addTo(mToolbars[ToolbarFile]);
    A(tr("Save As..."), tr("Saves the module to a new file"))
        .shortcut(QKeySequence::SaveAs)
        .triggers(lazyslotx(this, onSaveAs));
    SEP(); // -----------------------------------------------------------------
    A(tr("Export to WAV..."), tr("Exports the module to a WAV file"))
        .triggers(lazyslotx(this, onExportToWav));

    mRecentFiles.setup(menu);
    SEP(); // -----------------------------------------------------------------
    A(tr("Configuration..."), tr("Opens the configuration dialog"))
        .icon(icons::Config)
        .triggers(lazyslotx(this, onConfiguration))
        .addTo(mToolbars[ToolbarFile]);
    SEP(); // -----------------------------------------------------------------
    A(tr("&Quit"), tr("Exits the application"))
        .shortcut(QKeySequence::Quit)
        .triggers(lazyslotx(this, close));

    // =================================================================== Edit
    MENU(tr("&Edit"));
    A(mDocument->undoGroup()->createUndoAction(this))
        .icon(icons::Undo)
        .shortcut(QKeySequence::Undo)
        .addTo(mToolbars[ToolbarEdit]);
    A(mDocument->undoGroup()->createRedoAction(this))
        .icon(icons::Redo)
        .shortcut(QKeySequence::Redo)
        .addTo(mToolbars[ToolbarEdit]);
    A(tr("Undo History..."), tr("Shows the Undo History for the current song"));
    SEP(); // -----------------------------------------------------------------
    mToolbars[ToolbarEdit]->addSeparator();
    A(tr("C&ut"), tr("Copies and deletes selection to the clipboard"))
        .icon(icons::Cut)
        .shortcut(QKeySequence::Cut)
        // .triggers(mPatternEditor, &PatternEditor::cut)
        .addTo(mToolbars[ToolbarEdit]);
    A(tr("&Copy"), tr("Copies selected rows to the clipboard"))
        .icon(icons::Copy)
        .shortcut(QKeySequence::Copy)
        // .triggers(mPatternEditor, &PatternEditor::copy)
        .addTo(mToolbars[ToolbarEdit]);
    A(tr("&Paste"), tr("Pastes contents at the cursor"))
        .icon(icons::Paste)
        .shortcut(QKeySequence::Paste)
        // .triggers(mPatternEditor, &PatternEditor::paste)
        .addTo(mToolbars[ToolbarEdit]);
    A(tr("Paste &Mix"),
      tr("Pastes contents at the cursor, merging with existing rows"))
        // .triggers(mPatternEditor, &PatternEditor::pasteMix)
        .shortcut(tr("Ctrl+M"));
    A(tr("&Insert Row"), tr("Inserts an empty row at the cursor"))
        // .triggers(mPatternEditor, &PatternEditor::insertRow)
        .shortcut(tr("Ins"));
    A(tr("&Erase"), tr("Erases selection contents"))
        // .triggers(mPatternEditor, &PatternEditor::erase)
        .shortcut(QKeySequence::Delete);
    SEP(); // -----------------------------------------------------------------
    A(tr("&Select All"), tr("Selects entire track/pattern"))
        // .triggers(mPatternEditor, &PatternEditor::selectAll)
        .shortcut(QKeySequence::SelectAll);
    SEP(); // -----------------------------------------------------------------
    {
        SUBMENU(tr("Transpose"));
        A(tr("Decrease Note"), tr("Decreases note/notes by 1 step"))
            // .triggers(mPatternEditor, &PatternEditor::decreaseNote)
            .shortcut(tr("Ctrl+F1"));
        A(tr("Increase Note"), tr("Increases note/notes by 1 step"))
            // .triggers(mPatternEditor, &PatternEditor::increaseNote)
            .shortcut(tr("Ctrl+F2"));
        A(tr("Decrease Octave"), tr("Decreases note/notes by 12 steps"))
            // .triggers(mPatternEditor, &PatternEditor::decreaseOctave)
            .shortcut(tr("Ctrl+F3"));
        A(tr("Increase Octave"), tr("Increases note/notes by 12 steps"))
            // .triggers(mPatternEditor, &PatternEditor::increaseOctave)
            .shortcut(tr("Ctrl+F4"));
        A(tr("Custom..."), tr("Transpose by a custom amount"))
            // .triggers(mPatternEditor, &PatternEditor::transpose)
            .shortcut(tr("Ctrl+T"));
    }
    A(tr("&Reverse"), tr("Reverses selected rows"))
        // .triggers(mPatternEditor, &PatternEditor::reverse)
        .shortcut(tr("Ctrl+R"));
    A(tr("Replace Instrument"),
      tr("Replaces all instrument columns with the current one"))
    // .triggers(mPatternEditor, &PatternEditor::replaceInstrument)
    ;
    SEP(); // -----------------------------------------------------------------
    A(tr("Grow Pattern"),
      tr("Grows a selection by adding spaces in between rows"))
        // .triggers(mPatternEditor, &PatternEditor::growPattern)
        .shortcut(tr("Ctrl+G"));
    A(tr("Shrink Pattern"),
      tr("Shrinks a selection by removing spaces in between rows"))
        // .triggers(mPatternEditor, &PatternEditor::shrinkPattern)
        .shortcut(tr("Ctrl+H"));
    SEP(); // -----------------------------------------------------------------
    A(tr("Key Repetition"), tr("Toggles key repetition for the pattern editor"))
        .checkable()
        .checked()
        // .toggles(mPatternEditor, &PatternEditor::setKeyRepeat)
        .store(mUi.actKeyRepeat)
        .addTo(mToolbars[ToolbarInput]);

    // ================================================================= Module
    MENU(tr("&Module"));
    QAction *prevAction;
    QAction *nextAction;
    A(tr("Previous song"), tr("Selects the previous song in the list"))
        .icon(icons::SongPrev)
        .store(prevAction);
    A(tr("Next song"), tr("Selects the next song in the list"))
        .icon(icons::SongNext)
        .store(nextAction);
    SEP(); // -----------------------------------------------------------------
    A(tr("Comments..."), tr("Edit/view the module's comments"))
        .triggers(lazyslotx(this, showComments));
    A(tr("Song List..."), tr("Edit the module's song list"))
        .triggers(lazyslotx(this, showSongListEditor));
    A(tr("Module Properties..."), tr("Edit module properties"))
        .triggers(lazyslotx(this, showModuleProperties));

    // =================================================================== Song
    MENU(tr("&Song"));
    A(tr("&Insert Order Row"),
      tr("Inserts a new order row at the current pattern"))
        // .triggers(mPatternModel, &PatternModel::insertOrder)
        // .store(mActionInsertOrder)
        .icon(icons::Add);
    A(tr("&Remove order row"), tr("Removes the order at the current pattern"))
        .icon(icons::Remove);
    A(tr("&Duplicate order row"),
      tr("Duplicates the order at the current pattern"))
        .icon(icons::Duplicate);
    A(tr("Move order &up"), tr("Moves the order up 1")).icon(icons::Up);
    A(tr("Move order dow&n"), tr("Moves the order down 1")).icon(icons::Down);
    A(tr("Increment order"), tr("Increments all selected order cells by 1"))
        .icon(icons::Increment);
    A(tr("Decrement order"), tr("Decrements all selected order cells by 1"))
        .icon(icons::Decrement);
    A(tr("Change all"), tr("Toggles change all tracks mode"))
        .icon(icons::AllTracks)
        .checkable();
    SEP(); // -----------------------------------------------------------------
    A(tr("Tempo calculator..."), tr("Shows the tempo calculator dialog"));

    // ============================================================= Instrument
    MENU(tr("&Instrument"));
    auto setupTableMenu = [](QMenu *menu, QString const &kind) {
        A(tr("&Add"), tr("Add a new %1").arg(kind)).icon(icons::Add);
        A(tr("&Remove"), tr("Removes the current %1").arg(kind))
            .icon(icons::Remove);
        A(tr("&Duplicate"), tr("Duplicates the current %1").arg(kind))
            .icon(icons::Duplicate);
        SEP();
        A(tr("&Import"), tr("Imports a %1 from a file").arg(kind))
            .icon(icons::Import);
        A(tr("E&xport"), tr("Exports the current %s to a file").arg(kind))
            .icon(icons::Export);
        SEP();
        A(tr("&Edit"), tr("Opens the editor for the current %1").arg(kind))
            .icon(icons::Edit);
    };
    setupTableMenu(menu, tr("instrument"));

    // =============================================================== Waveform
    MENU(tr("&Waveform"));
    setupTableMenu(menu, tr("waveform"));

    // ================================================================ Tracker
    MENU(tr("&Tracker"));
    A(tr("&Play"),
      tr("Resume playing or play the song from the current position"))
        .addTo(mToolbars[ToolbarTracker])
        .icon(icons::Play);
    A(tr("Play from start"), tr("Begin playback of the song from the start"))
        .addTo(mToolbars[ToolbarTracker])
        .shortcut(tr("F5"))
        .icon(icons::PlayStart);
    A(tr("Play at cursor"), tr("Begin playback from the cursor"))
        .addTo(mToolbars[ToolbarTracker])
        .shortcut(tr("F6"))
        .icon(icons::PlayCursor);
    A(tr("Step row"), tr("Play and hold the row at the cursor"))
        .addTo(mToolbars[ToolbarTracker])
        .shortcut(tr("F7"))
        .icon(icons::Step);
    A(tr("&Stop"), tr("Stop playing"))
        .addTo(mToolbars[ToolbarTracker])
        .shortcut(tr("F8"))
        .icon(icons::Stop);
    A("Pattern repeat", "Toggles pattern repeat mode")
        .addTo(mToolbars[ToolbarTracker])
        .checkable()
        .shortcut(tr("F9"))
        .store(mUi.actPatternRepeat)
        .icon(icons::Repeat);
    A(tr("Record"), tr("Toggles record mode"))
        .addTo(mToolbars[ToolbarTracker])
        .checkable()
        .shortcut(tr("Space"))
        .store(mUi.actRecord)
        .icon(icons::Record);
    mToolbars[ToolbarTracker]->addAction(prevAction);
    mToolbars[ToolbarTracker]->addAction(nextAction);
    A(tr("Follow-mode"), tr("Toggles follow mode"))
        .checkable()
        .checked()
        .shortcut(tr("ScrollLock"))
        .store(mUi.actFollowMode)
        .addTo(mToolbars[ToolbarTracker]);
    SEP(); // -----------------------------------------------------------------
    A(tr("Toggle channel output"),
      tr("Enables/disables sound output for the current track"))
        .shortcut(tr("F10"));
    A(tr("Solo"), tr("Solos the current track")).shortcut(tr("F11"));
    SEP(); // -----------------------------------------------------------------
    A(tr("Reset volume"), tr("Resets the APU global volume setting"));
    A(tr("&Kill sound"), tr("Immediately stops sound output"))
        .shortcut(tr("F12"));

    // =================================================================== View
    MENU(tr("&View"));
    A(tr("Side Bar"), tr("Toggles visibility of the Side Bar"))
        .addTo(mToolbars[ToolbarView])
        .checkable()
        .checked()
        .toggles(lazyslotx(mUi.sidebar, setVisible))
        .store(mUi.actShowSidebar)
        .icon(icons::Sidebar);
    A(tr("Data Bar"), tr("Toggles visibility of the Data Bar"))
        .addTo(mToolbars[ToolbarView])
        .checkable()
        .checked()
        .toggles(lazyslotx(mUi.databar, setVisible))
        .store(mUi.actShowDatabar)
        .icon(icons::Databar);
    A(tr("Audio Scope"), tr("Enables the audio oscilloscope in the Side Bar"))
        .checkable()
        .checked();
    A(tr("Status Bar"), tr("Toggles visibility of the Status Bar"))
        .checkable()
        .checked()
        .store(mUi.actShowStatusbar)
        .toggles(statusBar(), &QStatusBar::setVisible);
    SEP(); // -----------------------------------------------------------------
    {
        SUBMENU(tr("Toolbars"));
        for (auto toolbar : mToolbars) {
            menu->addAction(toolbar->toggleViewAction());
        }
    }
    SEP(); // -----------------------------------------------------------------
    A(tr("Reset layout"),
      tr("Rearranges all docks and toolbars to the default layout"));

    // =================================================================== Help
    MENU(tr("&Help"));
    A(tr("&Effects list..."), tr("Shows a list of all available effects"))
        .shortcut(tr("F1"));
    A(tr("User &manual..."), tr("Opens the online user manual"))
        .shortcut(tr("F2"));
    A(tr("Audio &diagnostics..."), tr("Shows the audio diagnostics dialog"));
    SEP(); // -----------------------------------------------------------------
    A(tr("&About"), tr("About this program"));
    A(tr("About &Qt"), tr("Shows information about Qt"))
        .triggers(&QApplication::aboutQt);

#undef A
#undef SEP
#undef MENU
#undef SUBMENU
}

void MainWindow::initStatusBar() {
    /* auto status = */ statusBar();
}

void MainWindow::initUi() {

    auto container = new QWidget;
    auto layout = new QHBoxLayout;

    auto newPlaceholder = [](QString text) -> QLabel * {
        auto result = new QLabel(text);
        result->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        return result;
    };

    mUi.sidebar = new Sidebar(mSongListModel, mSongModel);
    mUi.editor = newPlaceholder("Pattern Editor");
    mUi.databar = new QSplitter(Qt::Vertical);
    mUi.instruments = newPlaceholder("Instruments");
    mUi.waveforms = newPlaceholder("Waveforms");
    mUi.databar->addWidget(mUi.instruments);
    mUi.databar->addWidget(mUi.waveforms);
    mUi.hsplitter = new QSplitter(Qt::Horizontal);
    mUi.hsplitter->addWidget(mUi.editor);
    mUi.hsplitter->addWidget(mUi.databar);

    layout->addWidget(mUi.sidebar);
    layout->addWidget(mUi.hsplitter, 1);
    container->setLayout(layout);

    setCentralWidget(container);
}

void MainWindow::loadSettings() {
    Settings s(SettingsState, TU::cGroup);
    if (!restoreGeometry(s.value(lit::geometry).toByteArray())) {
        // use a default geometry of 3/4 the primary screen's size
        auto const availableGeometry =
            QApplication::primaryScreen()->availableGeometry();

        auto scaled = [](int x) -> int {
            return x * 3 / 4;
        };
        QRect geom(0, 0, scaled(availableGeometry.width()),
                   scaled(availableGeometry.height()));
        geom.moveTo(availableGeometry.center() - geom.center());
        setGeometry(geom);
    }
    auto setActionChecked = [](QAction *act, QVariant const &val) {
        if (!val.isNull() && val.canConvert<bool>()) {
            act->setChecked(val.toBool());
        }
    };
    restoreState(s.value(lit::state).toByteArray(), TU::cWindowStateVers);
    setActionChecked(mUi.actShowDatabar, s.value(TU::cKeyShowDatabar));
    setActionChecked(mUi.actShowSidebar, s.value(TU::cKeyShowSidebar));
    setActionChecked(mUi.actShowStatusbar, s.value(TU::cKeyShowStatusbar));
    setActionChecked(mUi.actKeyRepeat, s.value(TU::cKeyKeyRepeat));
    setActionChecked(mUi.actFollowMode, s.value(TU::cKeyFollowMode));
    setActionChecked(mUi.actPatternRepeat, s.value(TU::cKeyPatternRepeat));
    setActionChecked(mUi.actRecord, s.value(TU::cKeyRecord));
    auto setSpinValue = [](QSpinBox *spin, QVariant const &val) {
        if (!val.isNull() && val.canConvert<int>()) {
            spin->setValue(val.toInt());
        }
    };
    setSpinValue(mUi.inputOctaveSpin, s.value(TU::cKeyOctave));
    setSpinValue(mUi.inputEditStep, s.value(TU::cKeyEditStep));
    mUi.hsplitter->restoreState(s.value(TU::cKeySplitterH).toByteArray());
    mUi.databar->restoreState(s.value(TU::cKeySplitterV).toByteArray());
}

void MainWindow::saveSettings() {
    Settings s(SettingsState, TU::cGroup);
    s.setValue(lit::geometry, saveGeometry());
    s.setValue(lit::state, saveState(TU::cWindowStateVers));
    s.setValue(TU::cKeyShowSidebar, mUi.actShowSidebar->isChecked());
    s.setValue(TU::cKeyShowDatabar, mUi.actShowDatabar->isChecked());
    s.setValue(TU::cKeyShowStatusbar, mUi.actShowStatusbar->isChecked());
    s.setValue(TU::cKeyOctave, mUi.inputOctaveSpin->value());
    s.setValue(TU::cKeyEditStep, mUi.inputEditStep->value());
    s.setValue(TU::cKeyKeyRepeat, mUi.actKeyRepeat->isChecked());
    s.setValue(TU::cKeyFollowMode, mUi.actFollowMode->isChecked());
    s.setValue(TU::cKeyPatternRepeat, mUi.actPatternRepeat->isChecked());
    s.setValue(TU::cKeyRecord, mUi.actRecord->isChecked());
    s.setValue(TU::cKeySplitterV, mUi.databar->saveState());
    s.setValue(TU::cKeySplitterH, mUi.hsplitter->saveState());
}

#undef TU
