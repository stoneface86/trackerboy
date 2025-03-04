

import
  ./backend/[cgen, core, notes]


proc handleCtrlC() {.noconv.} =
  discard

setControlCHook(handleCtrlC)

static:
  writeHeader()
