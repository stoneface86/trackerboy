
#include "MainWindow.hpp"


MainWindow::MainWindow() :
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


    connect(actionNew_waveform, &QAction::triggered, mDocument, &ModuleDocument::addWaveform);
    connect(mWaveformListView, &QAbstractItemView::clicked, mWaveEditor, &WaveEditor::selectWaveform);
    connect(mWaveformListView, &QAbstractItemView::doubleClicked, this, &MainWindow::waveformDoubleClicked);
}


// open the wave editor with the selected index
void MainWindow::waveformDoubleClicked(const QModelIndex &index) {
    mWaveEditor->show();
}
