
import
  ./interop,
  std/[strformat],
  libtrackerboy/[data, version]

type
  
  BSlice* {.exportc.} = object
    len*: int
    data*: cstring

  BPanicCallback* {.exportc.} = proc(msg: BSlice) {.noconv, raises: [].}

  Core = object
    panicCallback: BPanicCallback
    tempStr: string
  
  Box*[T] = object
    ## Wraps a type T into a single-member object. This is typically used for
    ## exporting Nim arrays to C, to ensure that value-semantics are used in
    ## function parameters/return values.
    ## 
    data*: T



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
  result.len = s.len + 1
  result.data = s.cstring

proc slice*(s: cstring): BSlice =
  result.len = s.len + 1
  result.data = s

proc toString*(s: BSlice): string =
  gCore.tempStr.setLen(s.len.int)
  for i in 0..<s.len.int:
    gCore.tempStr[i] = s.data[i]
  result = gCore.tempStr

proc assign*(s: var string; slice: BSlice) =
  s.setLen(slice.len)
  for i, ch in pairs(slice.data):
    s[i] = ch

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

# C API

proc bInit*() {.front.} =
  gCore.panicCallback = nil

proc bDeinit*() {.front.} =
  reset(gCore)

proc bSetPanicCallback*(callback: BPanicCallback) {.front.} =
  gCore.panicCallback = callback

proc bVersion*(): ccstring {.front.} =
  result = ccstring(currentVersionString)

