
#include "tbb.h"
#include "forms/MainWindow.hpp"
#include "version.hpp"

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


#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>

struct Demangled {
    const char *val;

    Demangled(const char *name) {
        int status = -1;
        val = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        owned = status == 0;
        if (!owned) {
            val = name;
        }
    }
    ~Demangled() {
        if (owned) {
            free((void*)val);
        }
    }
  
private:
    bool owned;
};

#else

struct Demangled {
    const char *val;

    Demangled(const char *name) :
        val(name)
    {
    }
};

#endif

static constexpr auto cUncaughtExceptionMsg = 
    "Problem:      Unhandled C++ exception\n"
    "Exception:    %s\n"
    "Message:      %s";

//
// Custom QApplication subclass to catch any uncaught exception during notify
//
class Application final : public QApplication {
    Q_OBJECT

public:
    using QApplication::QApplication;

    virtual bool notify(QObject *receiver, QEvent *evt) override {
        try {
            return QApplication::notify(receiver, evt);
        } catch (std::exception const& except) {
            Demangled demangled(typeid(except).name());
            qFatal(cUncaughtExceptionMsg, demangled.val, except.what());
        }
    }
};


// Message handler ---

// globals
static QtMessageHandler gDefaultMessager; // default message handler
static std::unique_ptr<MainWindow> gMainWindow;

//
// custom message handler that passes any fatal message to the user before
// exiting.
//
static void trackerboyMessage(QtMsgType type, QMessageLogContext const& ctx,
                              QString const& msg) {
    if (type == QtFatalMsg && gMainWindow) {
        gMainWindow->panic(msg);
    }
    gDefaultMessager(type, ctx, msg);
}

//
// Backend panic handler. Just calls qFatal with the error message.
//
extern "C" void backendPanic(B::Slice msg) {
    qFatal(msg.data);
}

static auto const cAppName = "TrackerBoy";
static constexpr int cExitBadArguments = -1;


int main(int argc, char *argv[]) {

#ifndef QT_NO_INFO_OUTPUT
    QElapsedTimer timer;
    timer.start();
#endif

    B::NimMain();
    B::init();
    B::setPanicCallback(backendPanic);

    gDefaultMessager = qInstallMessageHandler(trackerboyMessage);

    Application app(argc, argv);
    QCoreApplication::setOrganizationName(cAppName);
    QCoreApplication::setApplicationName(cAppName);
    QCoreApplication::setApplicationVersion(cVersion);
    // use INI on all systems, much easier to edit by hand
    QSettings::setDefaultFormat(QSettings::IniFormat);

#define tr(str) QCoreApplication::translate("main", str)

    QCommandLineParser parser;
    parser.setApplicationDescription(tr("Game Boy music tracker"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("[module]", tr("(Optional) the module file to open"));

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

            return cExitBadArguments;
    }

    // register types for signals
    qRegisterMetaType<ChannelOutput::Flags>("ChannelOutput::Flags");
    qRegisterMetaType<PatternModel::CursorChangeFlags>("CursorChangeFlags");

    // create and show MainWindow
    gMainWindow = std::make_unique<MainWindow>();
    gMainWindow->show();

    if (!fileToOpen.isEmpty()) {
        QFileInfo info(fileToOpen);
        if (!info.exists()) {
            QMessageBox::critical(
                gMainWindow.get(),
                tr("File does not exist"),
                tr("The module could not be opened because the file does not exist")
            );
        } else if (!info.isFile()) {
            QMessageBox::critical(
                gMainWindow.get(),
                tr("Invalid filename"),
                tr("The module could not be opened because it is not a file")
            );
        } else {
            gMainWindow->openFile(fileToOpen);
        }
    }


#ifndef QT_NO_INFO_OUTPUT
    qInfo() << "Launch time:" << timer.elapsed() << "ms";
#endif

    auto const code = app.exec();

    B::uninit();
    return code;
}

#include "main.moc"
