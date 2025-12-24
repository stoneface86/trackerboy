
#include "backend.hxx"

#include "core/icons.hxx"
#include "forms/MainWindow.hxx"
#include "version.hxx"

#include <QApplication>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QtDebug>

#include <cstdio>

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>

struct Demangled {
    const char *val;

    explicit Demangled(const char *name) {
        int status = -1;
        val = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        owned = status == 0;
        if (!owned) {
            val = name;
        }
    }
    ~Demangled() {
        if (owned) {
            free((void *)val);
        }
    }

private:
    bool owned;
};

#else

struct Demangled {
    const char *val;

    Demangled(const char *name)
        : val(name) {}
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

    bool notify(QObject *receiver, QEvent *evt) override {
        try {
            return QApplication::notify(receiver, evt);
        } catch (std::exception const &except) {
            Demangled const demangled(typeid(except).name());
            qFatal(cUncaughtExceptionMsg, demangled.val, except.what());
        }
    }
};

// Message handler ---

class Messenger {
    QtMessageHandler _defaultMessenger;
    MainWindow *_mainWindow;
    bool _handlingMessage;

    inline static Messenger *_instance = nullptr;

    static void handler(QtMsgType const type, QMessageLogContext const &ctx,
                        QString const &msg) {
        _instance->handlerImpl(type, ctx, msg);
    }

    void handlerImpl(QtMsgType const type, QMessageLogContext const &ctx,
                     QString const &msg) {
        if (!_handlingMessage) {
            _handlingMessage = true;
            if (type == QtFatalMsg && _mainWindow) {
                _mainWindow->panic(msg);
            }
            _defaultMessenger(type, ctx, msg);
            _handlingMessage = false;
        }
    }

public:
    explicit Messenger()
        : _defaultMessenger(qInstallMessageHandler(handler))
        , _mainWindow(nullptr)
        , _handlingMessage(false) {
        _instance = this;
    }

    ~Messenger() {
        _instance = nullptr;
        qInstallMessageHandler(nullptr);
    }

    void setWindow(MainWindow *win) { _mainWindow = win; }
};

//
// Backend panic handler. Just calls qFatal with the error message.
//
static void backendPanic(BSlice const msg) {
    qFatal("%s", msg.data);
}

static auto const cAppName = "TrackerBoy";
static constexpr int cExitBadArguments = -1;

int main(int argc, char *argv[]) {

#ifndef QT_NO_INFO_OUTPUT
    QElapsedTimer timer;
    timer.start();
#endif

    bNimMain();
    bInit();
    bSetPanicCallback(backendPanic);

    Messenger messenger;

    Application const app(argc, argv);
    // Application::setOrganizationName(cAppName);
    Application::setApplicationName(cAppName);
    Application::setApplicationDisplayName(cAppName);
    Application::setApplicationVersion(cVersion);
    Application::setWindowIcon(icons::getApp());
    // use INI on all systems, much easier to edit by hand
    QSettings::setDefaultFormat(QSettings::IniFormat);

#define tr(str) QCoreApplication::translate("main", str)

    QCommandLineParser parser;
    parser.setApplicationDescription(tr("Game Boy music tracker"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("[module]",
                                 tr("(Optional) the module file to open"));

    parser.process(app);

    QString fileToOpen;
    switch (auto const positionals = parser.positionalArguments();
            positionals.size()) {
    case 0:
        break;
    case 1:
        fileToOpen = positionals[0];
        break;
    default:
        // we could just only take the first argument and ignore the rest,
        // but I prefer to be strict
        fputs("too many arguments given\n", stderr);
        fputs(qPrintable(parser.helpText()), stderr);

        return cExitBadArguments;
    }

    // create and show MainWindow
    auto const win = new MainWindow;
    messenger.setWindow(win);
    win->show();

    if (!fileToOpen.isEmpty()) {
        if (QFileInfo const info(fileToOpen); !info.exists()) {
            QMessageBox::critical(win, tr("File does not exist"),
                                  tr("The module could not be opened because "
                                     "the file does not exist"));
        } else if (!info.isFile()) {
            QMessageBox::critical(
                win, tr("Invalid filename"),
                tr("The module could not be opened because it is not a file"));
        } else {
            win->openFile(fileToOpen);
        }
    }

#ifndef QT_NO_INFO_OUTPUT
    qInfo() << "Launch time:" << timer.elapsed() << "ms";
#endif

    auto const code = Application::exec();

    messenger.setWindow(nullptr);
    delete win;
    bDeinit();
    return code;
}

#include "main.moc"
