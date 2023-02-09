
import ../trackerboy/platforms
import utils
import std/[os, strutils, strformat]

const qtconfTesting {.booldefine.} = false

when qtconfTesting:
    import std/macros

const qtTarget = 6

template trace(echoParams: varargs[typed, `$`]) =
    when qtconfTesting:
        static:
            unpackVarargs(debugEcho, echoParams)
    else:
        discard

type
    QtVersionPart = enum
        qtMajor,
        qtMinor,
        qtPatch
    QtVersion = array[QtVersionPart, int]

    QMake = object
        cmd: string
        qmakeVersion: string
        qtVersion: QtVersion

{.push compileTime.}

func isValid(q: QMake): bool =
    result = q.cmd != ""

proc query(qmake: QMake, param: string): string =
    const cache = "0"
    result = gorge(&"{qmake.cmd} -query {param}", "", cache).strip


proc findQMake(cmd: string): QMake =
    const cache = "0"
    let (output, exitcode) = gorgeEx(&"{cmd} -v", "", cache)
    if exitcode == 0 and output != "":
        result.cmd = cmd
        result.qmakeVersion = result.query("QMAKE_VERSION")
        let version = result.query("QT_VERSION")
        let parts = version.split('.')
        if parts.len == 3:
            var i = 0
            for dest in result.qtVersion.mitems:
                dest = parseInt(parts[i])
                inc i

proc findQMake(): QMake =
    for cmd in [getEnv("QMAKE"), "qmake", "qmake6"]:
        let qmake = findQMake(cmd)
        if qmake.isValid() and qmake.qtVersion[qtMajor] == qtTarget:
            return qmake

const qmake = findQMake()
trace "QMake: ", qmake

when not qmake.isValid():
    {. error: "Could not find Qt" .}

const
    qtLibs = qmake.query("QT_INSTALL_LIBS")
    qtHeaders = qmake.query("QT_INSTALL_HEADERS")

trace "QT_INSTALL_LIBS: ", qtLibs
trace "QT_INSTALL_HEADERS: ", qtHeaders

const qtConfig* = block:
    var res: CompilerConfig
    when @pLinux:
        res.passc(
            "-std=gnu++17",
            &"-I{qtHeaders}",
            "-fPIC"  # needed on linux for some reason
        )
        res.passl(
            &"-L{qtLibs}",
            &"-lQt{qtTarget}Core",
            &"-lQt{qtTarget}Gui",
            &"-lQt{qtTarget}Widgets"
        )
    elif @pMac:
        res.passc(
            "-std=c++17",
            &"-I{qtHeaders}"
        )
        res.passl(
            &"-F{qtLibs}",
            "-framework QtCore",
            "-framework QtGui",
            "-framework QtWidgets"
        )
    elif @pWindows:
        res.passc(
            "/std:c++17",
            "/permissive-",
            "/Zc:__cplusplus",
            "/utf-8",
            &"/I{qtHeaders}"
        )
    res.passDefines(
        "QT_CORE_LIB",
        "QT_GUI_LIB",
        "QT_WIDGETS_LIB"
    )

    when defined(release):
        res.passDefines(
            "QT_NO_DEBUG",
            "QT_NO_INFO_OUTPUT",
            "QT_NO_DEBUG_OUTPUT"
        )

    res

{.pop.}
