
import qstring
export qstring

{. push header: "<QtCore/qglobal.h>" .}

type
    qsizetype* {. importcpp: "qsizetype" .} = csize_t
    qint8* {. importcpp: "qint8" .} = uint8

proc qPrintable*(str: QString): cstring
    {. importcpp: "qPrintable(@)" .}

{. pop .}
