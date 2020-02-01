/*
** File: src/instedit/instedit.cpp
**
** Main routine for the instedit (instrument editor) program
** This program is mainly for demonstration of trackerboy instruments
** It is not to be included in the main program, most of the stuff here
** is just prototype code.
*/

#include <QApplication>
#include "portaudio.h"

#include "InstEditor.hpp"




int main(int argc, char *argv[]) {

    int exitcode = 0;

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return 1;
    }


    QApplication app(argc, argv);
    instedit::InstEditor editor;
    editor.show();
    editor.setWindowTitle("Instrument Editor");
    exitcode = app.exec();


    Pa_Terminate();
    return exitcode;
}
