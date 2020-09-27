
#include <QApplication>
#include <QMessageBox>
#include "portaudio.h"

#include "audio.hpp"

#include "forms/MainWindow.hpp"

int main(int argc, char *argv[]) {

    int code = 0;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return 1;
    }

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("stoneface86");
    QCoreApplication::setApplicationName("Trackerboy");

    audio::DeviceTable& deviceTable = audio::DeviceTable::instance();
    if (deviceTable.isEmpty()) {
        QMessageBox::critical(
            nullptr,
            "Trackerboy",
            "The application cannot start. There are no available sound devices."
        );
        code = 1; 
    } else {

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
    }
    
    Pa_Terminate();
    return code;
}
