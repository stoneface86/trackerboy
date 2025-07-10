
#include "forms/MainWindow.hxx"
#include "core/icons.hxx"
#include "utils/actions.hxx"
#include "utils/connectutils.hxx"

#include "forms/SongListEditor.hxx"
#include "model/SongListModel.hxx"

#include <QApplication>
#include <QMenuBar>
#include <QScreen>
#include <QSettings>

#define TU MainWindowTU
namespace TU {

constexpr auto cGroup = "MainWindow";
constexpr auto cKeyGeometry = "geometry";
constexpr auto cKeySongListEditorSize = "songListEditorSize";

} // namespace TU

MainWindow::MainWindow()
    : QMainWindow()
    , mRecentFiles()
    , mDocument(new Document(this))
    , mSongListModel(new SongListModel(mDocument, this))
    , mSongListEditor(nullptr)
    , mModuleProperties{}
    , mComments{} {

    lazyconnect(mDocument, modifiedChanged, this, setWindowModified);

    QSettings settings;
    settings.beginGroup(TU::cGroup);
    if (!restoreGeometry(settings.value(TU::cKeyGeometry).toByteArray())) {
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
    initMenuBar();

    ColorTheme theme;
    theme.colors[0] = qRgb(208, 208, 247);
    theme.colors[1] = qRgb(113, 113, 191);
    theme.colors[2] = qRgb(51, 51, 102);
    theme.colors[3] = qRgb(31, 31, 61);
    icons::generate(theme);
    updateIcons();
}

void MainWindow::openFile(QString const &path) {
    Q_UNUSED(path)
}

void MainWindow::panic(QString const &msg) {
    Q_UNUSED(msg)
}

void MainWindow::showSongListEditor() {
    if (mSongListEditor == nullptr) {
        mSongListEditor = new SongListEditor(mSongListModel, this);
        // restore previous sizing
        QSettings settings;
        settings.beginGroup(TU::cGroup);
        auto const size = settings.value(TU::cKeySongListEditorSize);
        if (!size.isNull()) {
            mSongListEditor->resize(size.toSize());
        }
        connectLambda(mSongListEditor, finished, this, [this](int result) {
            if (result == QDialog::Accepted) {
                mSongListEditor->applyChanges(*mDocument);
            } else {
                mSongListEditor->revertChanges();
            }
            QSettings settings;
            settings.beginGroup(TU::cGroup);
            settings.setValue(TU::cKeySongListEditorSize,
                              mSongListEditor->size());
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
    for (auto menuAct : menuBar()->actions()) {
        for (auto act : menuAct->menu()->actions()) {
            auto const data = getData(act);
            if (data.icon != -1) {
                act->setIcon(icons::get((icons::Icons)data.icon));
            }
        }
    }
}

void MainWindow::initMenuBar() {

    auto const menubar = menuBar();
    QMenu *menu{};

#define A(...) buildAction(menu, __VA_ARGS__)

    // File
    mToolbarFile = icons::largeToolBar(this);
    menu = menubar->addMenu(tr("&File"));
    A(tr("&New"), tr("Create a new module"))
        .icon(icons::New)
        .shortcut(QKeySequence::New)
        .triggers(this, &MainWindow::onNew)
        .addTo(mToolbarFile);
    A(tr("&Open"), tr("Opens a module from a file"))
        .icon(icons::Open)
        .shortcut(QKeySequence::Open)
        .triggers(this, &MainWindow::onOpen)
        .addTo(mToolbarFile);
    A(tr("&Save"), tr("Saves the module"))
        .icon(icons::Save)
        .shortcut(QKeySequence::Save)
        .triggers(this, &MainWindow::onSave)
        .addTo(mToolbarFile);
    A(tr("Save As..."), tr("Saves the module to a new file"))
        .shortcut(QKeySequence::SaveAs)
        .triggers(this, &MainWindow::onSaveAs);
    menu->addSeparator();
    A(tr("Export to WAV..."), tr("Exports the module to a WAV file"))
        .triggers(this, &MainWindow::onExportToWav);

    mRecentFiles.setup(menu);
    menu->addSeparator();
    A(tr("Configuration..."), tr("Opens the configuration dialog"))
        .icon(icons::Config)
        .triggers(this, &MainWindow::onConfiguration)
        .addTo(mToolbarFile);
    menu->addSeparator();
    A(tr("&Quit"), tr("Exits the application"))
        .shortcut(QKeySequence::Quit)
        .triggers(this, &MainWindow::close);

    // Edit
    mToolbarEdit = icons::largeToolBar(this);
    menu = menubar->addMenu(tr("&Edit"));
    A(mDocument->undoGroup()->createUndoAction(this))
        .icon(icons::Undo)
        .shortcut(QKeySequence::Undo)
        .addTo(mToolbarEdit);
    A(mDocument->undoGroup()->createRedoAction(this))
        .icon(icons::Redo)
        .shortcut(QKeySequence::Redo)
        .addTo(mToolbarEdit);
    menu->addSeparator();
    mToolbarEdit->addSeparator();
    A(tr("C&ut"), tr("Copies and deletes selection to the clipboard"))
        .icon(icons::Cut)
        .shortcut(QKeySequence::Cut)
        // .triggers(mPatternEditor, &PatternEditor::cut)
        .addTo(mToolbarEdit);
    A(tr("&Copy"), tr("Copies selected rows to the clipboard"))
        .icon(icons::Copy)
        .shortcut(QKeySequence::Copy)
        // .triggers(mPatternEditor, &PatternEditor::copy)
        .addTo(mToolbarEdit);
    A(tr("&Paste"), tr("Pastes contents at the cursor"))
        .icon(icons::Paste)
        .shortcut(QKeySequence::Paste)
        // .triggers(mPatternEditor, &PatternEditor::paste)
        .addTo(mToolbarEdit);
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
    menu->addSeparator();
    A(tr("&Select All"), tr("Selects entire track/pattern"))
        // .triggers(mPatternEditor, &PatternEditor::selectAll)
        .shortcut(QKeySequence::SelectAll);

    // Module
    menu = menubar->addMenu(tr("&Module"));
    A(tr("Comments..."), tr("Edit/view the module's comments"))
        .triggers(this, &MainWindow::showComments);
    A(tr("Song List..."), tr("Edit the module's song list"))
        .triggers(this, &MainWindow::showSongListEditor);
    A(tr("Module Properties..."), tr("Edit module properties"))
        .triggers(this, &MainWindow::showModuleProperties);

    // Song
    menu = menubar->addMenu(tr("&Song"));

    // Instrument
    menu = menubar->addMenu(tr("&Instrument"));

    // Waveform
    menu = menubar->addMenu(tr("&Waveform"));

    // Tracker
    menu = menubar->addMenu(tr("&Tracker"));

    // View
    menu = menubar->addMenu(tr("&View"));

    // Help
    menu = menubar->addMenu(tr("&Help"));

    addToolBar(mToolbarFile);
    addToolBar(mToolbarEdit);

#undef A
}

#undef TU
