
import std/os
import ../gui/qguiapplication

type
    QApplicationArgs* = object
        ## QApplication requires that the lifetime of argv exceeds the lifetime
        ## of the QApplication instance, so this utility object maintains
        ## ownership of an argv, converted from a seq[string].
        argc*: cint
        argv*: cstringArray

proc `=destroy`*(app: var QApplicationArgs) =
    if app.argv != nil:
        deallocCStringArray(app.argv)
        app.argv = nil

{. push header: "<QtWidgets/QApplication>" .}

type
    QApplication* {. importcpp: "QApplication", pure .} = object of QGuiApplication

proc init*(_: typedesc[QApplication], argc: var cint, argv: cstringArray): QApplication
    {. importcpp: "QApplication(@)", constructor .}

proc exec*(app: var QApplication): cint
    {. importcpp: "#.exec()" .}

{.pop.}

proc init*(_: typedesc[QApplication], args: var QApplicationArgs): QApplication =
    QApplication.init(args.argc, args.argv)

proc init*(_: typedesc[QApplicationArgs], args: openArray[string]): QApplicationArgs =
    let argvSeq = block:
        var res = @[getAppFilename()]
        res.add(args)
        res
    
    result = QApplicationArgs(
        argv: allocCStringArray(argvSeq),
        argc: argvSeq.len.cint
    )
