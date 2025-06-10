
#include "forms/MainWindow.hxx"
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

void MainWindow::initMenuBar() {

    auto const menubar = menuBar();
    QMenu *menu{};

    // File
    menu = menubar->addMenu(tr("&File"));

    // Edit
    menu = menubar->addMenu(tr("&Edit"));

    // Module
    menu = menubar->addMenu(tr("&Module"));
    menu->addAction(tr("Comments..."), this, &MainWindow::showComments);
    menu->addAction(tr("Song List..."), this, &MainWindow::showSongListEditor);
    menu->addAction(tr("Module Properties..."), tr("Ctrl+P"), this,
                    &MainWindow::showModuleProperties);

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
}

#undef TU
