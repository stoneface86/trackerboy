
#include "MainWindow.hpp"

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow() :
    mModuleFileDialog(new QFileDialog(this)),
    mDocument(new ModuleDocument(this)),
    mInstrumentEditor(nullptr),
    mWaveEditor(nullptr),
    QMainWindow()
{
    setupUi(this);

    setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);

    mWaveformListView->setModel(mDocument->waveListModel());

    mInstrumentEditor = new InstrumentEditor(this);
    mWaveEditor = new WaveEditor(mDocument, this);

    mModuleFileDialog->setNameFilter(tr("Trackerboy Module (*.tbm)"));
    mModuleFileDialog->setWindowModality(Qt::WindowModal);
    mModuleFileDialog->setAcceptMode(QFileDialog::AcceptSave);

    
    connect(actionNew_waveform, &QAction::triggered, mDocument, &ModuleDocument::addWaveform);
    connect(mWaveformListView, &QAbstractItemView::clicked, mWaveEditor, &WaveEditor::selectWaveform);
    connect(mWaveformListView, &QAbstractItemView::doubleClicked, this, &MainWindow::waveformDoubleClicked);

    connect(mDocument, &ModuleDocument::modifiedChanged, this, &QMainWindow::setWindowModified);
    setFilename("");

    // Actions
    #define connectAction(action, slot) connect(action, &QAction::triggered, this, &MainWindow::slot)
    connectAction(actionNew, fileNew);
    connectAction(actionOpen, fileOpen);
    connectAction(actionSave, fileSave);
    connectAction(actionSaveAs, fileSaveAs);
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    if (maybeSave()) {
        //writeSettings();
        evt->accept();
    } else {
        evt->ignore();
    }
}

// SLOTS ---------------------------------------------------------------------

// open the wave editor with the selected index
void MainWindow::waveformDoubleClicked(const QModelIndex &index) {
    mWaveEditor->show();
}

void MainWindow::updateWindowTitle() {
    setWindowTitle(QString("%1[*] - Trackerboy").arg(mDocumentName));
}

// action slots

void MainWindow::fileNew() {
    if (maybeSave()) {
        setFilename("");
        mDocument->clear();
    }
}

void MainWindow::fileOpen() {
    if (maybeSave()) {
        mModuleFileDialog->setFileMode(QFileDialog::FileMode::ExistingFile);
        if (mModuleFileDialog->exec() == QDialog::Accepted) {
            QString filename = mModuleFileDialog->selectedFiles().first();
            auto error = mDocument->open(filename);
            if (error == trackerboy::FormatError::none) {
                setFilename(filename);
            }
        }
    }
}

bool MainWindow::fileSave() {
    if (mFilename.isEmpty()) {
        return fileSaveAs();
    } else {
        return mDocument->save(mFilename);
    }
}

bool MainWindow::fileSaveAs() {
    mModuleFileDialog->setFileMode(QFileDialog::FileMode::AnyFile);
    if (mModuleFileDialog->exec() != QDialog::Accepted) {
        return false;
    }
    QString filename = mModuleFileDialog->selectedFiles().first();
    if (mDocument->save(filename)) {
        setFilename(filename);
        return true;
    }

    return false;
}


bool MainWindow::maybeSave() {
    if (!mDocument->isModified()) {
        return true;
    }

    auto const result = QMessageBox::warning(
        this,
        tr("Trackerboy"),
        tr("Save changes to \"%1\"?").arg(mDocumentName),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

    switch (result) {
        case QMessageBox::Save:
            return fileSave();
        case QMessageBox::Cancel:
            return false;
        default:
            break;
    }

    return true;
}

void MainWindow::setFilename(QString filename) {
    mFilename = filename;
    if (filename.isEmpty()) {
        mDocumentName = "Untitled";
    } else {
        QFileInfo info(filename);
        mDocumentName = info.fileName();
    }
    updateWindowTitle();
}

