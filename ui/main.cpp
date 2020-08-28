
#include <QApplication>
#include "MainWindow.hpp"


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Trackerboy");
    QCoreApplication::setApplicationName("Trackerboy");

    MainWindow win;
    win.show();

    return app.exec();
}
