
import
  std/[macros, strformat],
  libtrackerboy/[data, version]

type

  BSlice* {.exportc.} = object
    len*: csizet
    data*: cstring

  BPanicCallback* = proc(msg: BSlice) {.noconv, raises: [].}

  BStr* {.exportc.} = object
    owner*: ref string
    len*: csizet
    data*: cstring

  Core = object
    panicCallback: BPanicCallback
    tempStr: string

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
  ## The proc definition is modified so that it has the exportc and noconv
  ## pragmas.
  ##
  expectKind(procDef, nnkProcDef)
  procDef.addPragma(ident("noconv"))
  procDef.addPragma(ident("exportc"))
  procDef.addPragma(ident("used"))
  result = procDef

proc assign(b: var BStr; s: string) =
  b.owner[] = s
  b.len = csizet(s.len)
  b.data = cstring(b.owner[])

proc bstr*(s: string): BStr =
  result.owner = new(string)
  GcRef(result.owner)
  result.assign(s)

proc bstr*(s: cstring): BStr =
  result.len = csizet(s.len)
  result.data = s

# C API

proc NimMain() {. importc, noconv .}

proc bInit*() {.front.} =
  NimMain()
  gCore.panicCallback = nil

proc bUninit*() {.front.} =
  discard

proc bSetPanicCallback*(callback: BPanicCallback) {.front.} =
  gCore.panicCallback = callback

proc bVersion*(): cstring {.front.} =
  result = cstring(currentVersionString)

proc bStrDestroy*(str: var BStr) {.front.} =
  if str.owner != nil:
    GcUnref(str.owner)
    str.owner = nil

proc bStrNew*(): BStr {.front.} =
  new(result.owner)
  GcRef(result.owner)

proc bStrSet*(str: var BStr; data: cstring) {.front.} =
  str.assign($data)

