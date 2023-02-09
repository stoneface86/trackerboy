
import ../core/qcoreapplication

{. push header: "QtGui/qguiapplication.h" .}

type
    QGuiApplication* {.importcpp: "QGuiApplication", pure.} = object of QCoreApplication

{. pop .}