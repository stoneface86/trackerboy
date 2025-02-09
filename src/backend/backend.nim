
proc handleCtrlC() {.noconv.} =
  discard

setControlCHook(handleCtrlC)
