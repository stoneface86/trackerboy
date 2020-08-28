
#include <QApplication>
#include "WaveEditor.hpp"


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    WaveEditor win;
    win.show();

    return app.exec();
}
