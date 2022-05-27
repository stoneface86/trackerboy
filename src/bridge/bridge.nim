
import trackerboy as tb

const hasPrintStackTrace = not defined(release)

when hasPrintStackTrace and defined(windows):
    # windows.h needs to be included before debugapi.h
    {.emit: """
    #include <windows.h>
    #include <debugapi.h>
    """.}
    proc OutputDebugStringA(str: cstring) {.importc, stdcall, dynlib: "Kernel32".}
    proc IsDebuggerPresent(): cint {.importc, stdcall, dynlib: "Kernel32".}

type
    Version {.exportcpp.} = object
        major: cint
        minor: cint
        patch: cint

    # Generic wrapper object, datatypes in the trackerboy library are not
    # exported to C/CPP so we export the Box instead
    Box[T] = object
        data: T

    Module {.exportcpp.} = Box[tb.Module]

    NimRef[T] {.importcpp, header: "NimRef.cpp".} = object

    # const cstring, type is generated as 'const char*' instead of 'char*'
    ccstringImpl {.importcpp: "const char*".} = cstring
    ccstring = distinct ccstringImpl

func toVersion(v: tb.Version): Version =
    Version(
        major: v.major.cint,
        minor: v.minor.cint,
        patch: v.patch.cint
    )

proc initNimRef[T](r: ref T): NimRef[T]
    {.importcpp: "NimRef<'*0>(@)", constructor, header: "NimRef.cpp".}

template exportRef(T: typedesc): untyped =
    # provide GcRef and GcUnref functions for NimRef<T>
    proc GcRef(t: ref T) = GC_ref(t)
    proc GcUnref(t: ref T) = GC_unref(t)
    # instantiate a NimRef template for T
    {.emit: "template class NimRef<" & astToStr(T) & ">;".}

{.push exportcpp, noconv.} # <-------------------- C++ Interface --------------

Module.exportRef

proc getExceptionName(e: ref Exception): ccstring = e.name.ccstring
proc getExceptionMsg(e: ref Exception): ccstring = e.msg.ccstring

when hasPrintStackTrace:
    proc printStackTrace(e: ref Exception) =
        let stackTrace = e.getStackTrace()
        when defined(windows):
            if IsDebuggerPresent() == 0:
                echo stackTrace
            else:
                OutputDebugStringA(stackTrace.cstring)
        else:
            echo stackTrace

proc getAppVersion*(): Version =
    tb.appVersion.toVersion

proc getLibVersion*(): Version =
    tb.libVersion.toVersion

proc newModule*(): NimRef[Module] =
    var m = (ref Module)(data: tb.Module.init())
    initNimRef(m)

proc module_clear*(t: ref Module) =
    t[].data = tb.Module.init()
