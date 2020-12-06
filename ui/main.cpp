
#include "forms/MainWindow.hpp"
#include "Trackerboy.hpp"

#include <QApplication>
#include <QFontDatabase>
#include <QMessageBox>

#include <memory>


int main(int argc, char *argv[]) {

    int code = 0;

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Trackerboy");
    QCoreApplication::setApplicationName("Trackerboy");

    // add the default font for the pattern editor
    QFontDatabase::addApplicationFont(":/CascadiaMono.ttf");

    std::unique_ptr<Trackerboy> trackerboy(new Trackerboy());
    
    // initialize audio
    auto result = trackerboy->miniaudio.init();
    if (result != MA_SUCCESS) {
        return 1;
    }


    std::unique_ptr<MainWindow> win(new MainWindow(*trackerboy));
    win->show();

    // don't use a catch-all in debug builds
    // this way the debugger can tell us exactly what happened

    // release builds will display the error (if possible) and
    // will attempt to save a copy of the module

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
            win.get(),
            "Error",
            "An error has occurred. The application will close now."
        );

    }
    #endif

    return code;
}
