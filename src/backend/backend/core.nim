
import
  std/[macros, strformat],
  libtrackerboy/[data, version]

type
  BSlice* {.exportc: "Slice".} = object
    len*: csizet
    data*: cstring

  PanicCallback* {.exportc.} = proc(msg: BSlice) {.noconv, raises: [].}

  Core = object
    panicCallback: PanicCallback
    tempStr: string

  ccstring* {. importc: "const char *" .} = distinct cstring
    ## Immutable version of cstring

var gCore: Core

template useCenum*() =
  {. pragma: cenum, size: sizeof(cint) .}

func describe(ex: ref Exception): string {. raises: [] .} =
  try:
    result = &"""
Problem:      Unhandled Nim exception
Exception:    {ex.name}
Message:      {ex.msg}
StackTrace:
"""
    if ex.trace.len() == 0:
      result.add("  <unavailable>\n")
    else:
      for st in ex.trace:
        result.add(&"  {st.filename}({st.line}) {st.procname}\n")
  except ValueError:
    discard

proc slice*(s: string): BSlice =
  result.len = csizet(s.len + 1)
  result.data = s.cstring

proc slice*(s: cstring): BSlice =
  result.len = csizet(s.len + 1)
  result.data = s

proc toString*(s: BSlice): string =
  gCore.tempStr.setLen(s.len.int)
  for i in 0..<s.len.int:
    gCore.tempStr[i] = s.data[i]
  result = gCore.tempStr


proc handleException*(ex: ref Exception) {.raises: [].} =
  ## General purpose exception handler for any unhandled exception.
  ##
  if gCore.panicCallback != nil:
    let details = describe(ex)
    gCore.panicCallback(slice(details))
  quit(1)

template canPanic*(body) =
  ## Catches any unhandled exception in `body` and handles it via
  ## [handleException].
  ##
  try:
    body
  except Exception as e:
    handleException(e)

macro front*(procDef) =
  ## Custom pragma that makes procs available to the front-end.
  ##
  ## The proc definition is modified so that it has the following pragmas:
  ##  * exportc
  ##  * noconv
  ##  * used
  ##  * raises: []
  ##
  expectKind(procDef, nnkProcDef)
  procDef.addPragma(ident("noconv"))
  procDef.addPragma(ident("exportcpp"))
  procDef.addPragma(ident("used"))
  procDef.addPragma(newTree(nnkExprColonExpr, ident("raises"), newTree(nnkBracket)))
  result = procDef

# C API

proc init*() {.front.} =
  gCore.panicCallback = nil

proc uninit*() {.front.} =
  reset(gCore)

proc setPanicCallback*(callback: PanicCallback) {.front.} =
  gCore.panicCallback = callback

let versionStrCopy = ccstring(currentVersionString)
proc version*(): ccstring {.front.} =
  result = versionStrCopy


