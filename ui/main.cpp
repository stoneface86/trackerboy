
#include "core/Trackerboy.hpp"
#include "forms/MainWindow.hpp"

#include <QApplication>
#include <QDateTime>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QtDebug>

#include <QFile>
#include <QTextStream>

#include <chrono>
#include <memory>
#include <new>

constexpr int EXIT_BAD_ALLOC = 1;
constexpr int EXIT_MINIAUDIO = 2;

//
// fuction pointer for the default qt message handler. We need to use a variable here
// since it's definition is not part of the Qt public API.
//
static QtMessageHandler DEFAULT_MESSAGE_HANDLER;

//
// Custom message handler that logs to file and to stderr/console via Qt's default handler
//
static void messageHandler(QtMsgType type, QMessageLogContext const& context, QString const& txt) {

    // temporary, for release we will log to the application data directory
    QFile outfile("log.txt");
    outfile.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream stream(&outfile);
    stream << QDateTime::currentDateTime().toString(QStringLiteral("yyyy/MM/dd hh:mm:ss"));

    stream << " [";
    switch (type) {
        case QtDebugMsg:
            stream << "DEBUG";
            break;
        case QtWarningMsg:
            stream << "WARNING";
            break;
        case QtCriticalMsg:
            stream << "CRITICAL";
            break;
        case QtFatalMsg:
            stream << "FATAL";
            // the default handler will do the abort
            break;
        case QtInfoMsg:
            stream << "INFO";
            break;
        default:
            break;
    }

    stream << "] " << txt << Qt::endl;
    
    DEFAULT_MESSAGE_HANDLER(type, context, txt);
}


int main(int argc, char *argv[]) {

    int code;

    #ifndef QT_NO_INFO_OUTPUT
    QElapsedTimer timer;
    timer.start();
    #endif

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Trackerboy");
    QCoreApplication::setApplicationName("Trackerboy");

    Q_INIT_RESOURCE(fonts);
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(images);



    // use a custom message handler for logging to file
    DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(messageHandler);

    // add the default font for the pattern editor
    QFontDatabase::addApplicationFont(":/CascadiaMono.ttf");

    std::unique_ptr<Trackerboy> trackerboy(new Trackerboy());
    
    // initialize audio
    auto result = trackerboy->miniaudio.init();
    if (result != MA_SUCCESS) {
        qCritical() << "could not initialize miniaudio:" << ma_result_description(result);
        return EXIT_MINIAUDIO;
    }


    std::unique_ptr<MainWindow> win(new MainWindow(*trackerboy));
    win->show();

    #ifndef QT_NO_INFO_OUTPUT
    qInfo() << "Launch time: " << timer.elapsed() << " ms";
    #endif

    try {
        code = app.exec();
    } catch (const std::bad_alloc &) {
        qCritical() << "out of memory";
        return EXIT_BAD_ALLOC;
    }

    return code;
}
