
import qglobal

{. push header: "<QtCore/qlist.h>" .}

type
    QList*[T] {. importcpp: "QList", pure.} = object {.inheritable.}

proc `[]`*[T](this: QList[T], i: qsizetype): T
    {. importcpp: "#[@]" .}

proc size*[T](this: QList[T]): qsizetype
    {. importcpp: "#.size()" .}

{. pop .}
