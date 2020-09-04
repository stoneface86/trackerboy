
#include "MainWindow.hpp"


MainWindow::MainWindow() :
    mInstrumentEditor(nullptr),
    QMainWindow()
{
    setupUi(this);

    setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);


    mInstrumentEditor = new InstrumentEditor(this);
    mInstrumentEditor->show();
}
