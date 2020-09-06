
#include <QApplication>
#include "MainWindow.hpp"
#include "InstrumentEditor.hpp"

int main(int argc, char *argv[]) {

    int code = 0;
    
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Trackerboy");
    QCoreApplication::setApplicationName("Trackerboy");

    MainWindow *win = new MainWindow();
    win->show();

    code = app.exec();

    delete win;
    

    return code;
}
