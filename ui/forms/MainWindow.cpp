
#include "MainWindow.hpp"
#include "Tileset.hpp"

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QScreen>

#include <array>

#pragma warning(push, 0)
#include "designer/ui_MainWindow.h"
#pragma warning(pop)

constexpr int TOOLBAR_ICON_WIDTH = 24;
constexpr int TOOLBAR_ICON_HEIGHT = 24;

constexpr int TOOLBAR_ICON_NEW = 0;
constexpr int TOOLBAR_ICON_OPEN = 1;
constexpr int TOOLBAR_ICON_SAVE = 2;


MainWindow::MainWindow() :
    mUi(new Ui::MainWindow()),
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
    mUi->setupUi(this);
    readSettings();

    mConfigDialog = new ConfigDialog(*mConfig, this);

    mWaveEditor = new WaveEditor(*mWaveModel, this);
    mInstrumentEditor = new InstrumentEditor(*mInstrumentModel, *mWaveModel, *mWaveEditor, this);
    

    mModuleFileDialog->setNameFilter(tr("Trackerboy Module (*.tbm)"));
    mModuleFileDialog->setWindowModality(Qt::WindowModal);

    Tileset tileset(QImage(":/icons/toolbar.png"), TOOLBAR_ICON_WIDTH, TOOLBAR_ICON_HEIGHT);
    QList<QToolBar*> toolbars = { mUi->toolbarFile, mUi->toolbarOrder, mUi->toolbarTracker };
    
    for (int i = 0, iconIndex = 0; i != toolbars.size(); ++i) {
        auto toolbar = toolbars.at(i);
        auto actions = toolbar->actions();
        for (int j = 0; j != actions.size(); ++j) {
            auto action = actions.at(j);
            if (action->isSeparator()) {
                continue;
            }
            action->setIcon(tileset.getIcon(iconIndex));
            ++iconIndex;
        }
    }
    
    connect(mDocument, &ModuleDocument::modifiedChanged, this, &QMainWindow::setWindowModified);
    setFilename("");

    // Actions
    #define connectAction(action, slot) connect(mUi->action, &QAction::triggered, this, &MainWindow::slot)
    // File
    connectAction(actionNew, fileNew);
    connectAction(actionOpen, fileOpen);
    connectAction(actionSave, fileSave);
    connectAction(actionSaveAs, fileSaveAs);
    connectAction(actionQuit, close);
    connect(mUi->actionConfiguration, &QAction::triggered, mConfigDialog, &QDialog::show);
    // Waveform
    //connect(actionEdit_waveform, &QAction::triggered, mWaveEditor, &WaveEditor::show);

    QApplication::connect(mUi->actionAboutQt, &QAction::triggered, &QApplication::aboutQt);

    auto wavePiano = mWaveEditor->piano();
    connect(wavePiano, &PianoWidget::keyDown, mRenderer, &Renderer::previewWaveform);
    connect(wavePiano, &PianoWidget::keyUp, mRenderer, &Renderer::stopPreview);

    auto instPiano = mInstrumentEditor->piano();
    connect(instPiano, &PianoWidget::keyDown, mRenderer, &Renderer::previewInstrument);
    connect(instPiano, &PianoWidget::keyUp, mRenderer, &Renderer::stopPreview);


    mUi->instrumentTableForm->init(mInstrumentModel, mInstrumentEditor, "Ctrl+I");
    // add the context menu for instruments list view to our menubar
    auto menu = mUi->instrumentTableForm->menu();
    menu->setTitle("Instrument");
    mUi->menubar->insertMenu(mUi->menuTracker->menuAction(), menu);

    mUi->waveTableForm->init(mWaveModel, mWaveEditor, "Ctrl+W");
    // same thing but for waveforms
    menu = mUi->waveTableForm->menu();
    menu->setTitle("Waveform");
    mUi->menubar->insertMenu(mUi->menuTracker->menuAction(), menu);

    QMenu *windowMenu = createPopupMenu();
    if (windowMenu != nullptr) {
        windowMenu->setTitle("Window");
        windowMenu->addSeparator();
        auto resetLayoutAction = windowMenu->addAction("Reset layout");
        connect(resetLayoutAction, &QAction::triggered, this, &MainWindow::windowResetLayout);
        mUi->menubar->insertMenu(mUi->menuHelp->menuAction(), windowMenu);
    }
}

MainWindow::~MainWindow() {
    delete mUi;
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

void MainWindow::windowResetLayout() {

    // this slot will reset all docks and toolbars to the default layout
    // since qt designer does not make this easy, we will do this here instead
    // of setting it up in the ui file.

    // when adding new docks to the ui file, add it to the list here

    // setup corners, left and right get both corners
    setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);
    setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);

    // toolbars
    // just add them in order to the top toolbar area

    std::array<QToolBar*, 3> toolbarArray = {
        mUi->toolbarFile,
        mUi->toolbarOrder,
        mUi->toolbarTracker
    };

    for (auto toolbar : toolbarArray) {
        addToolBar(Qt::ToolBarArea::TopToolBarArea, toolbar);
    }

    std::array<QDockWidget*, 5> dockArray = {
        mUi->dockSongProperties,
        mUi->dockModuleProperties,
        mUi->dockOrders,
        mUi->dockInstruments,
        mUi->dockWaveforms
    };

    // remove everything
    for (auto dock : dockArray) {
        dock->setFloating(false);
        removeDockWidget(dock);
    }

    // add everything back in the desired order

    // Note: a | means the docks are tabbed
    // left area: (dockSongProperties | dockModuleProperties) dockOrders
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, mUi->dockSongProperties);
    tabifyDockWidget(mUi->dockSongProperties, mUi->dockModuleProperties);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, mUi->dockOrders);

    // top area: dockInstruments dockWaveforms
    addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, mUi->dockInstruments);
    addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, mUi->dockWaveforms);
    
    // set visible
    for (auto dock : dockArray) {
        dock->setVisible(true);
    }

    // resize
    
    int topwidth = (width()) / 2;
    resizeDocks({ mUi->dockInstruments, mUi->dockWaveforms }, { topwidth, topwidth }, Qt::Horizontal);

    resizeDocks({ mUi->dockSongProperties, mUi->dockOrders }, { mUi->dockSongProperties->minimumHeight(), mUi->dockOrders->maximumHeight() }, Qt::Vertical);
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
    
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    if (geometry.isEmpty()) {
        // initialize window size to 3/4 of the screen's width and height
        // screen() was added at version 5.14
        const QRect availableGeometry = window()->screen()->availableGeometry();
        resize(availableGeometry.width() / 4 * 3, availableGeometry.height() / 4 * 3);
        move((availableGeometry.width() - width()) / 2,
            (availableGeometry.height() - height()) / 2);
        
    } else {
#else
    if (!geometry.isEmpty()) {
#endif
        restoreGeometry(geometry);
    }
    const QByteArray windowState = settings.value("windowState").toByteArray();
    if (windowState.isEmpty()) {
        // no saved window state, use the default
        // this way we don't have to fiddle with the layout in designer
        windowResetLayout();
    } else {
        restoreState(windowState);
    }
    
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
