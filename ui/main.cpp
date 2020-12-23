
#include "forms/MainWindow.hpp"
#include "Trackerboy.hpp"

#include <QApplication>
#include <QFontDatabase>
#include <QMessageBox>
#include <QtDebug>

#include <chrono>
#include <memory>
#include <new>

constexpr int EXIT_BAD_ALLOC = 1;
constexpr int EXIT_MINIAUDIO = 2;


int main(int argc, char *argv[]) {

    int code;

    #ifndef NDEBUG
    auto begin = std::chrono::steady_clock::now();
    #endif

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Trackerboy");
    QCoreApplication::setApplicationName("Trackerboy");

    // add the default font for the pattern editor
    QFontDatabase::addApplicationFont(":/CascadiaMono.ttf");

    std::unique_ptr<Trackerboy> trackerboy(new Trackerboy());
    
    // initialize audio
    auto result = trackerboy->miniaudio.init();
    if (result != MA_SUCCESS) {
        return EXIT_MINIAUDIO;
    }


    std::unique_ptr<MainWindow> win(new MainWindow(*trackerboy));
    win->show();

    #ifndef NDEBUG
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
    qDebug() << "Launch time: " << elapsed << " ms";
    #endif

    try {
        code = app.exec();
    } catch (const std::bad_alloc &) {
        return EXIT_BAD_ALLOC;
    }

    return code;
}
