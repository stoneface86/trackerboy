
import qobject, qstring

{. push header: "QtCore/qcoreapplication.h" .}

type
    QCoreApplication* {. importcpp: "QCoreApplication", pure.} = object of QObject

proc translate*(_: typedesc[QCoreApplication], 
    context, sourceText: cstring,
    disambiguation: cstring = nil,
    n: cint = -1
): QString
    {. importcpp: "QCoreApplication::translate(@)" .}

proc setOrganizationName*(_: typedesc[QCoreApplication], name: QString)
    {. importcpp: "QCoreApplication::setOrganizationName(@)" .}

proc setApplicationName*(_: typedesc[QCoreApplication], name: QString)
    {. importcpp: "QCoreApplication::setApplicationName(@)" .}

proc setApplicationVersion*(_: typedesc[QCoreApplication], version: QString)
    {. importcpp: "QCoreApplication::setApplicationVersion(@)" .}

{. pop .}
