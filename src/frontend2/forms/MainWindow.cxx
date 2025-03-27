
#include "forms/MainWindow.hxx"
#include "utils/connectutils.hxx"

#include <QMenuBar>


MainWindow::MainWindow() :
    QMainWindow(),
    mRecentFiles()
{
    mDocument = new Document(this);
    lazyconnect(mDocument, modifiedChanged, this, setWindowModified);
    
}

void MainWindow::openFile(QString const& path) {
    Q_UNUSED(path)
}

void MainWindow::panic(QString const& msg) {
    Q_UNUSED(msg)
}
