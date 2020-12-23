
#include "MainWindow.hpp"
#include "misc/IconManager.hpp"
#include "misc/utils.hpp"

#include <QApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QScreen>
#include <QMenuBar>
#include <QStatusBar>

#include <array>

#define setObjectNameFromDeclared(var) var.setObjectName(QStringLiteral(#var))
#define connectActionToThis(action, slot) connect(&action, &QAction::triggered, this, &MainWindow::slot)


constexpr int TOOLBAR_ICON_WIDTH = 16;
constexpr int TOOLBAR_ICON_HEIGHT = 16;

MainWindow::MainWindow(Trackerboy &trackerboy) :
    QMainWindow(),
    mApp(trackerboy),
    mFilename(),
    mDocumentName(),
    mConfigDialog(nullptr),
    mInstrumentEditor(nullptr),
    mWaveEditor(nullptr),
    mModuleFileDialog(),
    mToolbarFile(),
    mToolbarEdit(),
    mToolbarTracker(),
    mToolbarSong(),
    mSongCombo(),
    mDockInstruments(),
    mInstrumentWidget(trackerboy.instrumentModel, tr("Ctrl+I"), tr("instrument")),
    mDockWaveforms(),
    mWaveformWidget(trackerboy.waveModel, tr("Ctrl+W"), tr("wave")),
    mDockSongs(),
    mSongWidget(trackerboy.songModel),
    mDockSongProperties(),
    mSongPropertiesWidget(trackerboy.songModel),
    mDockModuleProperties(),
    mModulePropertiesWidget(),
    mDockOrders(),
    mOrderWidget(trackerboy.songModel.orderModel())

{
    setupUi();

    // read in application configuration
    mApp.config.readSettings();
    // apply the read in configuration
    onConfigApplied(Config::CategoryAll);

    // new documents have an empty string for a filename
    setFilename("");

    setWindowIcon(IconManager::getAppIcon());
}

MainWindow::~MainWindow() {
}

QMenu* MainWindow::createPopupMenu() {
    // we can't return a reference to mMenuWindow as QMainWindow will delete it
    // a new menu must be created
    auto menu = new QMenu(this);
    setupWindowMenu(*menu);
    return menu;
}

void MainWindow::closeEvent(QCloseEvent *evt) {
    if (maybeSave()) {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        evt->accept();
    } else {
        evt->ignore();
    }
}

void MainWindow::showEvent(QShowEvent *evt) {
    Q_UNUSED(evt);

    if (mDefaultLayoutState.isEmpty()) {

        QSettings settings;

        // restore geomtry from the last session
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


        // setup default layout
        // +-------------------------------------------------------------------------------+
        // | Toolbars                                                                      |
        // +-------------------------------------------------------------------------------+
        // |                   |                                   |                       |
        // | Song properties   |      Pattern Editor               | Instruments           |
        // |       +           |                                   |                       |
        // |     Songs         |                                   |                       |
        // |                   |                                   |                       |
        // +-------------------+                                   |                       |
        // |                   |                                   |                       |
        // | Module properties |                                   |                       |
        // |                   |                                   |                       | 
        // +-------------------+                                   |                       |
        // |                   |                                   |                       |
        // | Song Order        |                                   |                       |
        // |                   |                                   |                       |
        // |                   |                                   +-----------------------+
        // |                   |                                   |                       |
        // |                   |                                   | Waveforms             |
        // |                   |                                   |                       |
        // |                   |                                   |                       |
        // |                   |                                   |                       |
        // |                   |                                   |                       |
        // +-------------------------------------------------------------------------------+
        // Default locations for all dock widgets + toolbars
        // The user can move these or hide these however they like
        // This layout can be restored by invoking menu action Window | Reset Layout

        // setup corners, left and right get both corners
        setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
        setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);
        setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
        setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);


        addToolBar(Qt::TopToolBarArea, &mToolbarFile);
        mToolbarFile.show();

        addToolBar(Qt::TopToolBarArea, &mToolbarEdit);
        mToolbarEdit.show();

        addToolBar(Qt::TopToolBarArea, &mToolbarTracker);
        mToolbarTracker.show();

        addToolBar(Qt::TopToolBarArea, &mToolbarSong);
        mToolbarSong.show();

        addDockWidget(Qt::LeftDockWidgetArea, &mDockSongProperties);
        tabifyDockWidget(&mDockSongProperties, &mDockSongs);
        mDockSongs.show();
        mDockSongProperties.show();
        mDockSongProperties.raise();

        addDockWidget(Qt::LeftDockWidgetArea, &mDockModuleProperties);
        mDockModuleProperties.show();

        addDockWidget(Qt::LeftDockWidgetArea, &mDockOrders);
        mDockOrders.show();

        addDockWidget(Qt::RightDockWidgetArea, &mDockInstruments);
        mDockInstruments.show();

        addDockWidget(Qt::RightDockWidgetArea, &mDockWaveforms);
        mDockWaveforms.show();

        // resize

        int const w = width();
        int const h = height();

        // song properties and module properties get the minimum, stretch orders
        resizeDocks(
            { &mDockSongProperties, &mDockModuleProperties, &mDockOrders },
            { 0, 0, h },
            Qt::Vertical
        );

        // give instruments more height
        resizeDocks(
            { &mDockInstruments, &mDockWaveforms },
            { static_cast<int>(h * 0.75f), static_cast<int>(h * 0.25f) },
            Qt::Vertical
        );

        resizeDocks({ &mDockInstruments }, { static_cast<int>((w - mDockOrders.width()) * 0.375f) }, Qt::Horizontal);

        mDefaultLayoutState = saveState(); // save this default state so that the user can reset it

        // restore window state if it exists
        const QByteArray windowState = settings.value("windowState").toByteArray();
        if (!windowState.isEmpty()) {
            restoreState(windowState);
        }

    }

}

// SLOTS ---------------------------------------------------------------------

void MainWindow::updateWindowTitle() {
    setWindowTitle(QStringLiteral("%1[*] - Trackerboy").arg(mDocumentName));
}

// action slots

void MainWindow::onFileNew() {
    if (maybeSave()) {
        
        setModelsEnabled(false);
        mApp.document.clear();
        setModelsEnabled(true);

        setFilename("");

    }
}

void MainWindow::onFileOpen() {
    if (maybeSave()) {
        mModuleFileDialog.setFileMode(QFileDialog::FileMode::ExistingFile);
        mModuleFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        mModuleFileDialog.setWindowTitle("Open");
        if (mModuleFileDialog.exec() == QDialog::Accepted) {
            QString filename = mModuleFileDialog.selectedFiles().first();

            // disable models
            setModelsEnabled(false);

            auto error = mApp.document.open(filename);

            // renable models
            setModelsEnabled(true);

            if (error == trackerboy::FormatError::none) {
                setFilename(filename);
            }
        }
    }
}

bool MainWindow::onFileSave() {
    if (mFilename.isEmpty()) {
        return onFileSaveAs();
    } else {
        return mApp.document.save(mFilename);
    }
}

bool MainWindow::onFileSaveAs() {
    mModuleFileDialog.setFileMode(QFileDialog::FileMode::AnyFile);
    mModuleFileDialog.setAcceptMode(QFileDialog::AcceptSave);
    mModuleFileDialog.setWindowTitle("Save As");
    if (mModuleFileDialog.exec() != QDialog::Accepted) {
        return false;
    }
    QString filename = mModuleFileDialog.selectedFiles().first();
    if (mApp.document.save(filename)) {
        setFilename(filename);
        return true;
    }

    return false;
}

void MainWindow::onWindowResetLayout() {
    restoreState(mDefaultLayoutState);
}

void MainWindow::onConfigApplied(Config::Categories categories) {
    if (categories.testFlag(Config::CategorySound)) {
        auto &sound = mApp.config.sound();
        //auto rate = audio::SAMPLERATE_TABLE[sound.samplerate];
        mStatusSamplerate.setText(QString("%1 Hz").arg(sound.samplerate));

        mApp.renderer.setConfig(sound);
    }

    if (categories.testFlag(Config::CategoryAppearance)) {
        auto &appearance = mApp.config.appearance();

        setStyleSheet(QStringLiteral(R"stylesheet(
PatternEditor PatternGrid {
    font-family: %5;
    font-size: %6 px;
}

OrderWidget QTableView {
    background-color: %1;
    gridline-color: %2;
    color: %3;
    selection-color: %3;
    selection-background-color: %4;
    font-family: %5;
}

OrderWidget QTableView QTableCornerButton::section {
    background-color: %1;
    border-right: 1px solid %2;
    border-bottom: 1px solid %2;
    border-top: none;
    border bottom: none;
}

OrderWidget QTableView QHeaderView {
    background-color: %1;
    color: %3;
    font-family: %5;
}

OrderWidget QTableView QHeaderView::section {
    background-color: %1;
    border-right: 1px solid %2;
    border-bottom: 1px solid %2;
    border-top: none;
    border bottom: none;
}

)stylesheet").arg(
        mApp.colorTable[+Color::background].name(),
        mApp.colorTable[+Color::line].name(),
        mApp.colorTable[+Color::foreground].name(),
        mApp.colorTable[+Color::selection].name(),
        appearance.font.family(),
        QString::number(appearance.font.pixelSize())
        ));

        mPatternEditor->setColors(mApp.colorTable);
        mApp.orderModel.setRowColor(mApp.colorTable[+Color::row]);
    }

    mApp.config.writeSettings();
}

void MainWindow::showConfigDialog() {
    if (mConfigDialog == nullptr) {
        mConfigDialog = new ConfigDialog(mApp.config, this);
        mConfigDialog->resetControls();

        // configuration changed, apply settings
        connect(mConfigDialog, &ConfigDialog::applied, this, &MainWindow::onConfigApplied);
    }

    mConfigDialog->show();
}

void MainWindow::showInstrumentEditor() {
    if (mInstrumentEditor == nullptr) {
        mInstrumentEditor = new InstrumentEditor(mApp.instrumentModel, mApp.waveModel, this);

        // allow the instrument editor to show the wave editor
        connect(mInstrumentEditor, &InstrumentEditor::waveEditorRequested, this, &MainWindow::showWaveEditor);

        auto &instPiano = mInstrumentEditor->piano();
        connect(&instPiano, &PianoWidget::keyDown, &mApp.renderer, &Renderer::previewInstrument);
        connect(&instPiano, &PianoWidget::keyUp, &mApp.renderer, &Renderer::stopPreview);
    }
    mInstrumentEditor->show();
}

void MainWindow::showWaveEditor() {
    if (mWaveEditor == nullptr) {
        mWaveEditor = new WaveEditor(mApp.waveModel, this);
        auto &wavePiano = mWaveEditor->piano();
        connect(&wavePiano, &PianoWidget::keyDown, &mApp.renderer, &Renderer::previewWaveform);
        connect(&wavePiano, &PianoWidget::keyUp, &mApp.renderer, &Renderer::stopPreview);
    }
    mWaveEditor->show();
}


void MainWindow::statusSetInstrument(int index) {
    int id = (index == -1) ? 0 : mApp.instrumentModel.instrument(index)->id();
    mStatusInstrument.setText(QString("Instrument: %1").arg(id, 2, 16, QChar('0')));
}

void MainWindow::statusSetWaveform(int index) {
    int id = (index == -1) ? 0 : mApp.waveModel.waveform(index)->id();
    mStatusWaveform.setText(QString("Waveform: %1").arg(id, 2, 16, QChar('0')));
}

void MainWindow::statusSetOctave(int octave) {
    mStatusOctave.setText(QString("Octave: %1").arg(octave));
}

// PRIVATE METHODS -----------------------------------------------------------


bool MainWindow::maybeSave() {
    if (!mApp.document.isModified()) {
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
            return onFileSave();
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

void MainWindow::setModelsEnabled(bool enabled) {
    mApp.instrumentModel.setEnabled(enabled);
    mApp.songModel.setEnabled(enabled);
    mApp.waveModel.setEnabled(enabled);
}

void MainWindow::setupUi() {

    // CENTRAL WIDGET ========================================================

    // MainWindow expects this to heap-alloc'd as it will manually delete the widget
    mPatternEditor = new PatternEditor(mApp.songModel, mApp.colorTable);
    setCentralWidget(mPatternEditor);

    auto &patternActions = mPatternEditor->menuActions();

    // ACTIONS ===============================================================

    setupAction(mActionFileNew, "&New", "Create a new module", Icons::fileNew, QKeySequence::New);
    setupAction(mActionFileOpen, "&Open", "Open an existing module", Icons::fileOpen, QKeySequence::Open);
    setupAction(mActionFileSave, "&Save", "Save the module", Icons::fileSave, QKeySequence::Save);
    setupAction(mActionFileSaveAs, "Save &As...", "Save the module to a new file", QKeySequence::SaveAs);
    setupAction(mActionFileConfig, "&Configuration", "Change application settings", Icons::fileConfig);
    setupAction(mActionFileQuit, "&Quit", "Exit the application", QKeySequence::Quit);

    setupAction(mActionSongPrev, "&Previous song", "Selects the previous song in the list", Icons::previous);
    setupAction(mActionSongNext, "&Next song", "Selects the next song in the list", Icons::next);

    setupAction(mActionTrackerPlay, "&Play", "Play the song from the current pattern", Icons::trackerPlay);
    setupAction(mActionTrackerPlayPattern, "Play pattern", "Play and loop the current pattern", Icons::trackerPlayPattern);
    setupAction(mActionTrackerPlayStart, "Play from start", "Play the song starting at the first pattern");
    setupAction(mActionTrackerPlayCursor, "Play from cursor", "Play the song starting at the current cursor row");
    setupAction(mActionTrackerStop, "&Stop", "Stop playing", Icons::trackerStop);
    setupAction(mActionTrackerEditMode, "Toggle &edit mode", "Enables/disables edit mode", Icons::trackerEdit);
    setupAction(mActionTrackerToggleChannel, "Toggle channel output", "Enables/disables sound output for the current track");
    setupAction(mActionTrackerSolo, "Solo", "Solos the current track");

    setupAction(mActionWindowResetLayout, "Reset layout", "Rearranges all docks and toolbars to the default layout");

    setupAction(mActionHelpAbout, "&About", "About this program");
    setupAction(mActionHelpAboutQt, "About &Qt", "Shows information about Qt");

    // MENUS ==============================================================


    mMenuFile.setTitle(tr("&File"));
    mMenuFile.addAction(&mActionFileNew);
    mMenuFile.addAction(&mActionFileOpen);
    mMenuFile.addAction(&mActionFileSave);
    mMenuFile.addAction(&mActionFileSaveAs);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileConfig);
    mMenuFile.addSeparator();
    mMenuFile.addAction(&mActionFileQuit);

    mMenuEdit.setTitle(tr("&Edit"));
    mPatternEditor->setupMenu(mMenuEdit);

    mMenuSong.setTitle(tr("&Song"));
    mSongWidget.setupMenu(mMenuSong);
    mMenuSong.addSeparator();
    mMenuSong.addAction(&mActionSongPrev);
    mMenuSong.addAction(&mActionSongNext);

    mMenuOrder.setTitle(tr("&Order"));
    mOrderWidget.setupMenu(mMenuOrder);

    mMenuInstrument.setTitle(tr("&Instrument"));
    mInstrumentWidget.setupMenu(mMenuInstrument);

    mMenuWaveform.setTitle(tr("&Waveform"));
    mWaveformWidget.setupMenu(mMenuWaveform);

    mMenuTracker.setTitle(tr("&Tracker"));
    mMenuTracker.addAction(&mActionTrackerPlay);
    mMenuTracker.addAction(&mActionTrackerPlayPattern);
    mMenuTracker.addAction(&mActionTrackerPlayStart);
    mMenuTracker.addAction(&mActionTrackerPlayCursor);
    mMenuTracker.addAction(&mActionTrackerStop);
    mMenuTracker.addAction(&mActionTrackerEditMode);
    mMenuTracker.addSeparator();
    mMenuTracker.addAction(&mActionTrackerToggleChannel);
    mMenuTracker.addAction(&mActionTrackerSolo);

    mMenuWindow.setTitle(tr("Wi&ndow"));
    mMenuWindowToolbars.setTitle(tr("&Toolbars"));
    mMenuWindowToolbars.addAction(mToolbarFile.toggleViewAction());
    mMenuWindowToolbars.addAction(mToolbarEdit.toggleViewAction());
    mMenuWindowToolbars.addAction(mToolbarTracker.toggleViewAction());
    mMenuWindowToolbars.addAction(mToolbarSong.toggleViewAction());

    setupWindowMenu(mMenuWindow);

    mMenuHelp.setTitle(tr("&Help"));
    mMenuHelp.addAction(&mActionHelpAbout);
    mMenuHelp.addAction(&mActionHelpAboutQt);

    // MENUBAR ================================================================

    auto menubar = menuBar();
    menubar->addMenu(&mMenuFile);
    menubar->addMenu(&mMenuEdit);
    menubar->addMenu(&mMenuSong);
    menubar->addMenu(&mMenuOrder);
    menubar->addMenu(&mMenuInstrument);
    menubar->addMenu(&mMenuWaveform);
    menubar->addMenu(&mMenuTracker);
    menubar->addMenu(&mMenuWindow);
    menubar->addMenu(&mMenuHelp);

    // TOOLBARS ==============================================================

    QSize const iconSize(16, 16);

    mToolbarFile.setWindowTitle(tr("File"));
    mToolbarFile.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarFile);
    mToolbarFile.addAction(&mActionFileNew);
    mToolbarFile.addAction(&mActionFileOpen);
    mToolbarFile.addAction(&mActionFileSave);
    mToolbarFile.addSeparator();
    mToolbarFile.addAction(&mActionFileConfig);

    mToolbarEdit.setWindowTitle(tr("Edit"));
    mToolbarEdit.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarEdit);
    mToolbarEdit.addAction(&patternActions.undo);
    mToolbarEdit.addAction(&patternActions.redo);
    mToolbarEdit.addSeparator();
    mToolbarEdit.addAction(&patternActions.cut);
    mToolbarEdit.addAction(&patternActions.copy);
    mToolbarEdit.addAction(&patternActions.paste);

    mToolbarTracker.setWindowTitle(tr("Tracker"));
    mToolbarTracker.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarTracker);
    mToolbarTracker.addAction(&mActionTrackerPlay);
    mToolbarTracker.addAction(&mActionTrackerPlayPattern);
    mToolbarTracker.addAction(&mActionTrackerStop);
    mToolbarTracker.addAction(&mActionTrackerEditMode);


    mToolbarSong.setWindowTitle(tr("Song"));
    mToolbarSong.setIconSize(iconSize);
    setObjectNameFromDeclared(mToolbarSong);
    mToolbarSong.addAction(&mActionSongPrev);
    mToolbarSong.addAction(&mActionSongNext);
    mSongCombo.setModel(&mApp.songModel);
    mToolbarSong.addWidget(&mSongCombo);

    // DIALOGS ===============================================================

    mModuleFileDialog.setNameFilter(tr("Trackerboy Module (*.tbm)"));
    mModuleFileDialog.setWindowModality(Qt::WindowModal);

    // DOCKS =================================================================

    setObjectNameFromDeclared(mDockInstruments);
    mDockInstruments.setWindowTitle(tr("Instruments"));
    mDockInstruments.setWidget(&mInstrumentWidget);

    setObjectNameFromDeclared(mDockWaveforms);
    mDockWaveforms.setWindowTitle(tr("Waveforms"));
    mDockWaveforms.setWidget(&mWaveformWidget);

    setObjectNameFromDeclared(mDockSongs);
    mDockSongs.setWindowTitle(tr("Songs"));
    mDockSongs.setWidget(&mSongWidget);

    setObjectNameFromDeclared(mDockModuleProperties);
    mDockModuleProperties.setWindowTitle(tr("Module properties"));
    mDockModuleProperties.setWidget(&mModulePropertiesWidget);

    setObjectNameFromDeclared(mDockSongProperties);
    mDockSongProperties.setWindowTitle(tr("Song properties"));
    mDockSongProperties.setWidget(&mSongPropertiesWidget);

    setObjectNameFromDeclared(mDockOrders);
    mDockOrders.setWindowTitle(tr("Song order"));
    mDockSongs.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mDockOrders.setWidget(&mOrderWidget);

    

    // STATUSBAR ==============================================================

    auto statusbar = statusBar();
    #define addLabelToStatusbar(var, text) var.setText(QStringLiteral(text)); statusbar->addPermanentWidget(&var)
    
    addLabelToStatusbar(mStatusInstrument, "Instrument: 00");
    addLabelToStatusbar(mStatusWaveform, "Waveform: 00");
    addLabelToStatusbar(mStatusOctave, "Octave: 3");
    addLabelToStatusbar(mStatusFramerate, "59.7 FPS");
    addLabelToStatusbar(mStatusSpeed, "6.0 FPR");
    addLabelToStatusbar(mStatusTempo, "150 BPM");
    addLabelToStatusbar(mStatusElapsed, "00:00:00");
    addLabelToStatusbar(mStatusPos, "00 / 00");
    addLabelToStatusbar(mStatusSamplerate, "");


    // CONNECTIONS ============================================================

    connect(&mApp.document, &ModuleDocument::modifiedChanged, this, &QMainWindow::setWindowModified);

    // Actions

    // File
    connectActionToThis(mActionFileNew, onFileNew);
    connectActionToThis(mActionFileOpen, onFileOpen);
    connectActionToThis(mActionFileSave, onFileSave);
    connectActionToThis(mActionFileSaveAs, onFileSaveAs);
    connectActionToThis(mActionFileQuit, close);
    connectActionToThis(mActionWindowResetLayout, onWindowResetLayout);

    connectActionToThis(mActionFileConfig, showConfigDialog);
    //connect(&mActionFileConfig, &QAction::triggered, &mConfigDialog, &ConfigDialog::show);

    QApplication::connect(&mActionHelpAboutQt, &QAction::triggered, &QApplication::aboutQt);


    // song combobox in mSongToolbar
    connect(&mSongCombo, qOverload<int>(&QComboBox::currentIndexChanged), &mApp.songModel, qOverload<int>(&SongListModel::select));
    connect(&mApp.songModel, &SongListModel::currentIndexChanged, &mSongCombo, &QComboBox::setCurrentIndex);

    // statusbar

    connect(&mApp.instrumentModel, &InstrumentListModel::currentIndexChanged, this, &MainWindow::statusSetInstrument);
    connect(&mApp.waveModel, &WaveListModel::currentIndexChanged, this, &MainWindow::statusSetWaveform);

    // showEditor signal to each editor's show slot
    connect(&mInstrumentWidget, &TableForm::showEditor, this, &MainWindow::showInstrumentEditor);
    connect(&mWaveformWidget, &TableForm::showEditor, this, &MainWindow::showWaveEditor);

    

}

void MainWindow::setupWindowMenu(QMenu &menu) {
    menu.addAction(mDockInstruments.toggleViewAction());
    menu.addAction(mDockWaveforms.toggleViewAction());
    menu.addAction(mDockSongs.toggleViewAction());
    menu.addAction(mDockSongProperties.toggleViewAction());
    menu.addAction(mDockModuleProperties.toggleViewAction());
    menu.addAction(mDockOrders.toggleViewAction());

    menu.addSeparator();
    
    menu.addMenu(&mMenuWindowToolbars);
    
    menu.addSeparator();
    
    menu.addAction(&mActionWindowResetLayout);
}


//void MainWindow::setupAction(QAction &action, const char *text, const char *tooltip, QKeySequence const &seq) {
//    action.setText(tr(text));
//    action.setShortcut(seq);
//    action.setStatusTip(tr(tooltip));
//}
