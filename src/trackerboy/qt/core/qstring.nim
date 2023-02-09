
import ../../interop

{.push header: "QtCore/qstring.h" .}

type
    QString* {. importcpp: "QString" .} = object

proc init*(_: typedesc[QString]): QString
    {. importcpp: "QString()", constructor.}

proc init*(_: typedesc[QString], str: ccstring): QString
    {. importcpp: "QString(@)", constructor .}

proc QStringLiteral*(lit: cstring{lit}): QString
    {. importcpp: "QStringLiteral(@)" .}

template `%`*(lit: string{lit}): QString =
    QStringLiteral(lit)

{.pop.}
