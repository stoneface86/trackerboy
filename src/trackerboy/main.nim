
import qt/core/[qcoreapplication, qcommandlineparser, qglobal, qstring]
import qt/widgets/qapplication

import std/[os]

const nimblePkgVersion {.strdefine.} = ""

template mainTr(str: string): QString =
    QCoreApplication.translate("main", str)

{. emit: "#include <QtWidgets/QWidget>" .}

proc main*(): int =
    var args = QApplicationArgs.init(commandLineParams())
    var app = QApplication.init(args.argc, args.argv)
    let name = %"Trackerboy"
    QCoreApplication.setOrganizationName(name)
    QCoreApplication.setApplicationName(name)
    QCoreApplication.setApplicationVersion(QString.init(nimblePkgVersion))

    var parser = QCommandLineParser.init()
    parser.setApplicationDescription(mainTr("Game Boy music tracker"))
    parser.addHelpOption()
    parser.addVersionOption()
    parser.addPositionalArgument(
        %"[module_file]",
        mainTr("(Optional) the module file to open")
    )

    parser.process(app)

    var fileToOpen: QString
    let positionals = parser.positionalArguments()
    case positionals.size()
    of 0:
        discard
    of 1:
        fileToOpen = positionals[0]
    else:
        stderr.writeLine("too many arguments given")
        stderr.writeLine(qPrintable(parser.helpText()))
        return 1

    {. emit: """
QWidget win;
win.show();
    """ .}

    app.exec()

