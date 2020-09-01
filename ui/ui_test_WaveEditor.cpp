
#include <QApplication>
#include "WaveEditor.hpp"

#include "trackerboy/data/Module.hpp"


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    trackerboy::Module mod;
    auto &wtable = mod.waveTable();
    wtable.insert();

    WaveEditor win(mod);
    win.show();

    return app.exec();
}
