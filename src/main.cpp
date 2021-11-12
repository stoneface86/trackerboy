
#include "forms/MainWindow.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QFontDatabase>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QPointer>
#include <QStringBuilder>
#include <QtDebug>
#include <QTextStream>

#include <iostream>
#include <chrono>
#include <memory>
#include <new>
#include <cstdio>

#include "version.hpp"


#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>

static std::string demangle(const char* name) {
    int status = -1;

    std::unique_ptr<char, void(*)(void*)> demangled {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status == 0) ? demangled.get() : name;
}

#else

static std::string demangle(const char* name) {
    return name;
}

#endif




constexpr int EXIT_BAD_ARGUMENTS = -1;
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

    void setWindow(MainWindow *window) {
        mWindow = window;
    }


private:
    MessageHandler() :
        mDefaultHandler(qInstallMessageHandler(message))
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

//
// custom QApplication subclass that catches any uncaught exceptions
//
class Application final : public QApplication {

    Q_OBJECT

public:
    using QApplication::QApplication;

    virtual bool notify(QObject *receiver, QEvent *evt) override {
        try {
            return QApplication::notify(receiver, evt);
        } catch (std::bad_alloc &) {
            // out of memory, propagate it to main()
            throw;
        } catch (std::exception const& except) {
            // catch any uncaught exception here, so that we can attempt
            // to save the user's current module before aborting, as well
            // as logging the problem to the user.
            auto which = demangle(typeid(except).name());
            qFatal("[Uncaught exception]\nType: %s\nWhat: %s", which.c_str(), except.what());
            return true;
        }
    }

};



int main(int argc, char *argv[]) {

    int code;

    #ifndef QT_NO_INFO_OUTPUT
    QElapsedTimer timer;
    timer.start();
    #endif

    Application app(argc, argv);
    QCoreApplication::setOrganizationName("Trackerboy");
    QCoreApplication::setApplicationName("Trackerboy");
    QCoreApplication::setApplicationVersion(VERSION_STR);
    // use INI on all systems, much easier to edit by hand
    QSettings::setDefaultFormat(QSettings::IniFormat);

#define main_tr(str) QCoreApplication::translate("main", str)

    QCommandLineParser parser;
    parser.setApplicationDescription(main_tr("Game Boy music tracker"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("[module_file]", main_tr("(Optional) the module file to open"));

    parser.process(app);

    QString fileToOpen;
    auto const positionals = parser.positionalArguments();
    switch (positionals.size()) {
        case 0:
            break;
        case 1:
            fileToOpen = positionals[0];
            break;
        default:
            // we could just only take the first argument and ignore the rest
            // but I prefer to be strict
            fputs("too many arguments given\n", stderr);
            fputs(qPrintable(parser.helpText()), stderr);

            return EXIT_BAD_ARGUMENTS;
    }

    // register types for signals
    qRegisterMetaType<ChannelOutput::Flags>("ChannelOutput::Flags");
    qRegisterMetaType<PatternModel::CursorChangeFlags>("CursorChangeFlags");

    // instantiate the custom message handler for logging to file
    MessageHandler::instance();

    // add the default font for the pattern editor
    QFontDatabase::addApplicationFont(":/CascadiaMono.ttf");
   
    auto win = std::make_unique<MainWindow>();
    MessageHandler::instance().setWindow(win.get());
    win->show();

    if (!fileToOpen.isEmpty()) {
        QFileInfo info(fileToOpen);
        if (!info.exists()) {
            QMessageBox::critical(
                win.get(),
                main_tr("File does not exist"),
                main_tr("The module could not be opened because the file does not exist")
            );
        } else if (!info.isFile()) {
            QMessageBox::critical(
                win.get(),
                main_tr("Invalid filename"),
                main_tr("The module could not be opened because it is not a file")
            );
        } else {
            win->openFile(fileToOpen);
        }
    }


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

#include "main.moc"
