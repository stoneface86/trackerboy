
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
    , _recentFiles()
    , _document(new Document(this))
    , _songListModel(new SongListModel(_document, this))
    , _songModel(new SongModel(_document, _songListModel, this))
    , _songListEditor(nullptr)
    , _moduleProperties(nullptr)
    , _comments(nullptr)
    , _toolbars{}
    , _ui{} {

    lazyconnect(_document, modifiedChanged, this, setWindowModified);

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
    if (_songListEditor == nullptr) {
        _songListEditor = new SongListEditor(_songListModel, this);
        connectLambda(_songListEditor, finished, this, [this](int result) {
            if (result == QDialog::Accepted) {
                _songListEditor->applyChanges(*_document);
            } else {
                _songListEditor->revertChanges();
            }
        });
    }
    // TODO: Stop playback
    _songListEditor->open();
}

void MainWindow::showComments() {
    if (_comments == nullptr) {
        _comments = new CommentsDialog(_document, this);
    }
    _comments->show();
}

void MainWindow::showModuleProperties() {
    if (_moduleProperties == nullptr) {
        _moduleProperties = new ModulePropertiesDialog(this);
        connectLambda(_moduleProperties, accepted, this, [this]() {
            _moduleProperties->save(*_document);
        });
    }
    _moduleProperties->load(*_document);
    _moduleProperties->open();
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
        _toolbars[toolbar] = bar;
    };

    initToolBar(ToolbarFile, tr("File"));
    initToolBar(ToolbarEdit, tr("Edit"));
    initToolBar(ToolbarTracker, tr("Tracker"));
    initToolBar(ToolbarInput, tr("Input"));
    initToolBar(ToolbarView, tr("View"));
    {
        auto *bar = _toolbars[ToolbarInput];

        auto container = new QWidget;
        auto layout = new ToolbarLayout(QBoxLayout::LeftToRight);
        layout->addWidget(new QLabel(tr("Octave")));
        _ui.inputOctaveSpin = new QSpinBox;
        _ui.inputOctaveSpin->setValue(5);
        _ui.inputOctaveSpin->setRange(2, 8);
        layout->addWidget(_ui.inputOctaveSpin);
        layout->addWidget(new QLabel(tr("Edit Step")));
        _ui.inputEditStep = new QSpinBox;
        _ui.inputEditStep->setRange(1, 255);
        layout->addWidget(_ui.inputEditStep);
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
        .addTo(_toolbars[ToolbarFile]);
    A(tr("&Open"), tr("Opens a module from a file"))
        .icon(icons::Open)
        .shortcut(QKeySequence::Open)
        .triggers(lazyslotx(this, onOpen))
        .addTo(_toolbars[ToolbarFile]);
    A(tr("&Save"), tr("Saves the module"))
        .icon(icons::Save)
        .shortcut(QKeySequence::Save)
        .triggers(lazyslotx(this, onSave))
        .addTo(_toolbars[ToolbarFile]);
    A(tr("Save As..."), tr("Saves the module to a new file"))
        .shortcut(QKeySequence::SaveAs)
        .triggers(lazyslotx(this, onSaveAs));
    SEP(); // -----------------------------------------------------------------
    A(tr("Export to WAV..."), tr("Exports the module to a WAV file"))
        .triggers(lazyslotx(this, onExportToWav));

    _recentFiles.setup(menu);
    SEP(); // -----------------------------------------------------------------
    A(tr("Configuration..."), tr("Opens the configuration dialog"))
        .icon(icons::Config)
        .triggers(lazyslotx(this, onConfiguration))
        .addTo(_toolbars[ToolbarFile]);
    SEP(); // -----------------------------------------------------------------
    A(tr("&Quit"), tr("Exits the application"))
        .shortcut(QKeySequence::Quit)
        .triggers(lazyslotx(this, close));

    // =================================================================== Edit
    MENU(tr("&Edit"));
    A(_document->undoGroup()->createUndoAction(this))
        .icon(icons::Undo)
        .shortcut(QKeySequence::Undo)
        .addTo(_toolbars[ToolbarEdit]);
    A(_document->undoGroup()->createRedoAction(this))
        .icon(icons::Redo)
        .shortcut(QKeySequence::Redo)
        .addTo(_toolbars[ToolbarEdit]);
    A(tr("Undo History..."), tr("Shows the Undo History for the current song"));
    SEP(); // -----------------------------------------------------------------
    _toolbars[ToolbarEdit]->addSeparator();
    A(tr("C&ut"), tr("Copies and deletes selection to the clipboard"))
        .icon(icons::Cut)
        .shortcut(QKeySequence::Cut)
        // .triggers(mPatternEditor, &PatternEditor::cut)
        .addTo(_toolbars[ToolbarEdit]);
    A(tr("&Copy"), tr("Copies selected rows to the clipboard"))
        .icon(icons::Copy)
        .shortcut(QKeySequence::Copy)
        // .triggers(mPatternEditor, &PatternEditor::copy)
        .addTo(_toolbars[ToolbarEdit]);
    A(tr("&Paste"), tr("Pastes contents at the cursor"))
        .icon(icons::Paste)
        .shortcut(QKeySequence::Paste)
        // .triggers(mPatternEditor, &PatternEditor::paste)
        .addTo(_toolbars[ToolbarEdit]);
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
        .store(_ui.actKeyRepeat)
        .addTo(_toolbars[ToolbarInput]);

    // ================================================================= Module
    MENU(tr("&Module"));
    A(tr("Previous song"), tr("Selects the previous song in the list"))
        .icon(icons::SongPrev)
        .store(_ui.actSongPrev)
        .triggers(lazyslotx(this, onModulePrevSong));
    A(tr("Next song"), tr("Selects the next song in the list"))
        .icon(icons::SongNext)
        .store(_ui.actSongNext)
        .triggers(lazyslotx(this, onModuleNextSong));
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
        .addTo(_toolbars[ToolbarTracker])
        .icon(icons::Play);
    A(tr("Play from start"), tr("Begin playback of the song from the start"))
        .addTo(_toolbars[ToolbarTracker])
        .shortcut(tr("F5"))
        .icon(icons::PlayStart);
    A(tr("Play at cursor"), tr("Begin playback from the cursor"))
        .addTo(_toolbars[ToolbarTracker])
        .shortcut(tr("F6"))
        .icon(icons::PlayCursor);
    A(tr("Step row"), tr("Play and hold the row at the cursor"))
        .addTo(_toolbars[ToolbarTracker])
        .shortcut(tr("F7"))
        .icon(icons::Step);
    A(tr("&Stop"), tr("Stop playing"))
        .addTo(_toolbars[ToolbarTracker])
        .shortcut(tr("F8"))
        .icon(icons::Stop);
    A("Pattern repeat", "Toggles pattern repeat mode")
        .addTo(_toolbars[ToolbarTracker])
        .checkable()
        .shortcut(tr("F9"))
        .store(_ui.actPatternRepeat)
        .icon(icons::Repeat);
    A(tr("Record"), tr("Toggles record mode"))
        .addTo(_toolbars[ToolbarTracker])
        .checkable()
        .shortcut(tr("Space"))
        .store(_ui.actRecord)
        .icon(icons::Record);
    _toolbars[ToolbarTracker]->addAction(_ui.actSongPrev);
    _toolbars[ToolbarTracker]->addAction(_ui.actSongNext);
    A(tr("Follow-mode"), tr("Toggles follow mode"))
        .checkable()
        .checked()
        .shortcut(tr("ScrollLock"))
        .store(_ui.actFollowMode)
        .addTo(_toolbars[ToolbarTracker]);
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
        .addTo(_toolbars[ToolbarView])
        .checkable()
        .checked()
        .toggles(lazyslotx(_ui.sidebar, setVisible))
        .store(_ui.actShowSidebar)
        .icon(icons::Sidebar);
    A(tr("Data Bar"), tr("Toggles visibility of the Data Bar"))
        .addTo(_toolbars[ToolbarView])
        .checkable()
        .checked()
        .toggles(lazyslotx(_ui.databar, setVisible))
        .store(_ui.actShowDatabar)
        .icon(icons::Databar);
    A(tr("Audio Scope"), tr("Enables the audio oscilloscope in the Side Bar"))
        .checkable()
        .checked();
    A(tr("Status Bar"), tr("Toggles visibility of the Status Bar"))
        .checkable()
        .checked()
        .store(_ui.actShowStatusbar)
        .toggles(statusBar(), &QStatusBar::setVisible);
    SEP(); // -----------------------------------------------------------------
    {
        SUBMENU(tr("Toolbars"));
        for (auto toolbar : _toolbars) {
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

    updateSongSelectActions();
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

    _ui.sidebar = new Sidebar(_songListModel, _songModel);
    _ui.editor = newPlaceholder("Pattern Editor");
    _ui.databar = new QSplitter(Qt::Vertical);
    _ui.instruments = newPlaceholder("Instruments");
    _ui.waveforms = newPlaceholder("Waveforms");
    _ui.databar->addWidget(_ui.instruments);
    _ui.databar->addWidget(_ui.waveforms);
    _ui.hsplitter = new QSplitter(Qt::Horizontal);
    _ui.hsplitter->addWidget(_ui.editor);
    _ui.hsplitter->addWidget(_ui.databar);

    layout->addWidget(_ui.sidebar);
    layout->addWidget(_ui.hsplitter, 1);
    container->setLayout(layout);

    setCentralWidget(container);
    lazyconnect(_document, songChanged, this, updateSongSelectActions);
    lazyconnect(_songListModel, modelReset, this, updateSongSelectActions);
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
    setActionChecked(_ui.actShowDatabar, s.value(TU::cKeyShowDatabar));
    setActionChecked(_ui.actShowSidebar, s.value(TU::cKeyShowSidebar));
    setActionChecked(_ui.actShowStatusbar, s.value(TU::cKeyShowStatusbar));
    setActionChecked(_ui.actKeyRepeat, s.value(TU::cKeyKeyRepeat));
    setActionChecked(_ui.actFollowMode, s.value(TU::cKeyFollowMode));
    setActionChecked(_ui.actPatternRepeat, s.value(TU::cKeyPatternRepeat));
    setActionChecked(_ui.actRecord, s.value(TU::cKeyRecord));
    auto setSpinValue = [](QSpinBox *spin, QVariant const &val) {
        if (!val.isNull() && val.canConvert<int>()) {
            spin->setValue(val.toInt());
        }
    };
    setSpinValue(_ui.inputOctaveSpin, s.value(TU::cKeyOctave));
    setSpinValue(_ui.inputEditStep, s.value(TU::cKeyEditStep));
    _ui.hsplitter->restoreState(s.value(TU::cKeySplitterH).toByteArray());
    _ui.databar->restoreState(s.value(TU::cKeySplitterV).toByteArray());
}

void MainWindow::saveSettings() {
    Settings s(SettingsState, TU::cGroup);
    s.setValue(lit::geometry, saveGeometry());
    s.setValue(lit::state, saveState(TU::cWindowStateVers));
    s.setValue(TU::cKeyShowSidebar, _ui.actShowSidebar->isChecked());
    s.setValue(TU::cKeyShowDatabar, _ui.actShowDatabar->isChecked());
    s.setValue(TU::cKeyShowStatusbar, _ui.actShowStatusbar->isChecked());
    s.setValue(TU::cKeyOctave, _ui.inputOctaveSpin->value());
    s.setValue(TU::cKeyEditStep, _ui.inputEditStep->value());
    s.setValue(TU::cKeyKeyRepeat, _ui.actKeyRepeat->isChecked());
    s.setValue(TU::cKeyFollowMode, _ui.actFollowMode->isChecked());
    s.setValue(TU::cKeyPatternRepeat, _ui.actPatternRepeat->isChecked());
    s.setValue(TU::cKeyRecord, _ui.actRecord->isChecked());
    s.setValue(TU::cKeySplitterV, _ui.databar->saveState());
    s.setValue(TU::cKeySplitterH, _ui.hsplitter->saveState());
}

void MainWindow::updateSongSelectActions() {
    auto const currentSong = _document->song();
    _ui.actSongPrev->setEnabled(currentSong > 0);
    _ui.actSongNext->setEnabled(currentSong + 1 < _songListModel->rowCount());
}

void MainWindow::onModuleNextSong() {
    _document->selectSong(_document->song() + 1);
}

void MainWindow::onModulePrevSong() {
    _document->selectSong(_document->song() - 1);
}

#undef TU
