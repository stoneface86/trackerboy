
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
    , mSongListEditor(nullptr) {

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

#undef TU
