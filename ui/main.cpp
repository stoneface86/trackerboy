
#include <QApplication>
#include "portaudio.h"

#include "MainWindow.hpp"
#include "InstrumentEditor.hpp"

int main(int argc, char *argv[]) {

    int code = 0;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return 1;
    }
    
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Trackerboy");
    QCoreApplication::setApplicationName("Trackerboy");

    MainWindow *win = new MainWindow();
    win->show();

    code = app.exec();

    delete win;
    
    Pa_Terminate();
    return code;
}
