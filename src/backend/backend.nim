
{. push warning[UnusedImport]: off .}

import
  ./backend/[core, data, input, interop, notes]


proc handleCtrlC() {.noconv.} =
  discard

setControlCHook(handleCtrlC)

static:
  writeHeader()
