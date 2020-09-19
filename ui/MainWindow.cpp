
#include "MainWindow.hpp"

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QScreen>


MainWindow::MainWindow() :
    mModuleFileDialog(new QFileDialog(this)),
    mDocument(new ModuleDocument(this)),
    mInstrumentModel(new InstrumentListModel(*mDocument)),
    mWaveModel(new WaveListModel(*mDocument)),
    mConfig(new Config(this)),
    mConfigDialog(nullptr),
    mInstrumentEditor(nullptr),
    mWaveEditor(nullptr),
    mRenderer(new Renderer(*mDocument, *mInstrumentModel, *mWaveModel, this)),
    QMainWindow()
{
    setupUi(this);
    readSettings();

    mConfigDialog = new ConfigDialog(*mConfig, this);

    setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);

    mInstrumentEditor = new InstrumentEditor(this);
    mWaveEditor = new WaveEditor(*mWaveModel, this);

    mModuleFileDialog->setNameFilter(tr("Trackerboy Module (*.tbm)"));
    mModuleFileDialog->setWindowModality(Qt::WindowModal);

    auto __style = style();
    actionNew->setIcon(__style->standardIcon(QStyle::SP_FileIcon));
    actionOpen->setIcon(__style->standardIcon(QStyle::SP_DialogOpenButton));
    actionSave->setIcon(__style->standardIcon(QStyle::SP_DialogSaveButton));
    actionSaveAs->setIcon(__style->standardIcon(QStyle::SP_DialogSaveButton));
    
    connect(mDocument, &ModuleDocument::modifiedChanged, this, &QMainWindow::setWindowModified);
    setFilename("");

    // Actions
    #define connectAction(action, slot) connect(action, &QAction::triggered, this, &MainWindow::slot)
    // File
    connectAction(actionNew, fileNew);
    connectAction(actionOpen, fileOpen);
    connectAction(actionSave, fileSave);
    connectAction(actionSaveAs, fileSaveAs);
    connectAction(actionQuit, close);
    connect(actionConfiguration, &QAction::triggered, mConfigDialog, &QDialog::show);
    // Waveform
    //connect(actionEdit_waveform, &QAction::triggered, mWaveEditor, &WaveEditor::show);

    auto wavePiano = mWaveEditor->piano();
    connect(wavePiano, &PianoWidget::keyDown, mRenderer, &Renderer::previewWaveform);
    connect(wavePiano, &PianoWidget::keyUp, mRenderer, &Renderer::stopPreview);

    mInstrumentTableForm->init(mInstrumentModel, mInstrumentEditor);
    // add the context menu for instruments list view to our menubar
    auto menu = mInstrumentTableForm->menu();
    menu->setTitle("Instrument");
    mMenubar->insertMenu(mMenuTracker->menuAction(), menu);

    mWaveTableForm->init(mWaveModel, mWaveEditor);
    // same thing but for waveforms
    menu = mWaveTableForm->menu();
    menu->setTitle("Waveform");
    mMenubar->insertMenu(mMenuTracker->menuAction(), menu);
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    if (maybeSave()) {
        writeSettings();
        evt->accept();
    } else {
        evt->ignore();
    }
}

// SLOTS ---------------------------------------------------------------------

void MainWindow::updateWindowTitle() {
    setWindowTitle(QString("%1[*] - Trackerboy").arg(mDocumentName));
}

// action slots

void MainWindow::fileNew() {
    if (maybeSave()) {
        
        setModelsEnabled(false);
        mDocument->clear();
        setModelsEnabled(true);

        setFilename("");

    }
}

void MainWindow::fileOpen() {
    if (maybeSave()) {
        mModuleFileDialog->setFileMode(QFileDialog::FileMode::ExistingFile);
        mModuleFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        mModuleFileDialog->setWindowTitle("Open");
        if (mModuleFileDialog->exec() == QDialog::Accepted) {
            QString filename = mModuleFileDialog->selectedFiles().first();

            // disable models
            setModelsEnabled(false);

            auto error = mDocument->open(filename);

            // renable models
            setModelsEnabled(true);

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
    mModuleFileDialog->setAcceptMode(QFileDialog::AcceptSave);
    mModuleFileDialog->setWindowTitle("Save As");
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

void MainWindow::readSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
            (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
    restoreState(settings.value("windowState").toByteArray());
    mConfig->readSettings(settings);
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

void MainWindow::setModelsEnabled(bool enabled) {
    mWaveModel->setEnabled(enabled);
    mInstrumentModel->setEnabled(enabled);
}

void MainWindow::writeSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    mConfig->writeSettings(settings);
}
