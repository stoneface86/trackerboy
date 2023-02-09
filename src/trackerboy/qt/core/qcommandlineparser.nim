
import qcoreapplication, qstring, qstringlist
export qstring, qstringlist

{. push header: "QtCore/qcommandlineparser.h" .}

type
    QCommandLineParser* {. importcpp: "QCommandLineParser" .} = object

proc init*(_: typedesc[QCommandLineParser]): QCommandLineParser
    {. importcpp: "#()", constructor .}

proc setApplicationDescription*(p: var QCommandLineParser, desc: QString)
    {. importcpp: "#.setApplicationDescription(@)" .}

proc addHelpOption*(p: var QCommandLineParser)
    {. importcpp: "#.addHelpOption()" .}

proc addVersionOption*(p: var QCommandLineParser)
    {. importcpp: "#.addVersionOption()" .}

proc addPositionalArgument*(
    this: var QCommandLineParser,
    name, description: QString,
    syntax = QString.init()
) {. importcpp: "#.addPositionalArgument(@)" .}

proc helpText*(this: QCommandLineParser): QString
    {. importcpp: "#.helpText()" .}

proc process*(this: var QCommandLineParser, app: QCoreApplication)
    {. importcpp: "#.process(@)" .}

proc positionalArguments*(this: QCommandLineParser): QStringList
    {. importcpp: "#.positionalArguments()" .}

{.pop.}
