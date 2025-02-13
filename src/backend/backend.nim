

import
  ./backend/[core]


proc handleCtrlC() {.noconv.} =
  discard

setControlCHook(handleCtrlC)
