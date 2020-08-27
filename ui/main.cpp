
#include <QApplication>
//#include "MainWindow.hpp"
#include "WaveEditor.hpp"


int main(int argc, char *argv[]) {

    int exitcode = 0;



    QApplication app(argc, argv);
    //instedit::InstEditor editor;
    //editor.show();
    //editor.setWindowTitle("Instrument Editor");
    //MainWindow win;
    //win.show();
    WaveEditor win;
    win.show();


    exitcode = app.exec();


    return exitcode;
}
