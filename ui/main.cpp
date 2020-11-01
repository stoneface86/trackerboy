
#include <QApplication>
#include <QMessageBox>

#include "forms/MainWindow.hpp"

int main(int argc, char *argv[]) {

    int code = 0;

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("stoneface86");
    QCoreApplication::setApplicationName("Trackerboy");

    // the backend table lives here, so that its destruction is guaranteed to
    // occur after MainWindow
    //audio::BackendTable backendTable;

    MainWindow *win = new MainWindow();
    win->show();

    #ifdef NDEBUG
    try {
    #endif
        code = app.exec();
    #ifdef NDEBUG
    } catch (const std::exception &e) {
        (void)e;

        // TODO: attempt to save the current module
        // TODO: display the type of error and extra diagnostic info
        QMessageBox::critical(
            win,
            "Error",
            "An error has occurred. The application will close now."
        );

    }
    #endif

    delete win;

    return code;
}
