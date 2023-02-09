
import qstring, qlist
export qstring, qlist

{. push header: "<QtCore/qstringlist.h>" .}

type
    QStringList* {. importcpp: "QStringList" .} = object of QList[QString]

{. pop .}
