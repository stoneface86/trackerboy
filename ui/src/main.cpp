
#include "forms/MainWindow.hpp"

#include <QApplication>
#include <QDateTime>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QtDebug>

#include <QFile>
#include <QTextStream>
#include <QStringBuilder>
#include <QPointer>

#include <iostream>
#include <chrono>
#include <memory>
#include <new>

constexpr int EXIT_BAD_ALLOC = 1;

//
// Singleton class for a custom Qt message handler. This message handler wraps
// the default handler and calls MainWindow::panic for QtFatalMsg messages.
//
class MessageHandler {

public:

    static MessageHandler& instance() {
        static MessageHandler instance;
        return instance;
    }

    void registerHandler() {
        mDefaultHandler = qInstallMessageHandler(message);
    }

    void setWindow(MainWindow *window) {
        mWindow = window;
    }


private:
    MessageHandler() :
        mDefaultHandler(nullptr)
    {
    }

    static void message(QtMsgType type, QMessageLogContext const& context, QString const& txt) {
        instance()._message(type, context, txt);
    }

    void _message(QtMsgType type, QMessageLogContext const& context, QString const& txt) {

        if (type == QtFatalMsg && mWindow) {
            // force the window to save a copy of the module if needed
            mWindow->panic(txt);
        }
        
        // let qt's default handler do everything else
        mDefaultHandler(type, context, txt);
    }

    QPointer<MainWindow> mWindow;
    QtMessageHandler mDefaultHandler;

};

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
    Q_INIT_RESOURCE(stylesheets);

    // register types for signals
    qRegisterMetaType<ChannelOutput::Flags>("ChannelOutput::Flags");
    qRegisterMetaType<PatternModel::CursorChangeFlags>("CursorChangeFlags");

    // use a custom message handler for logging to file
    MessageHandler::instance().registerHandler();

    // add the default font for the pattern editor
    QFontDatabase::addApplicationFont(":/CascadiaMono.ttf");
   
    std::unique_ptr<MainWindow> win(new MainWindow);
    MessageHandler::instance().setWindow(win.get());
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
