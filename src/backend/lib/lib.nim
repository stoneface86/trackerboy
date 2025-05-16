
{. push warning[UnusedImport]: off .}

import
  ./backend/[
    core, 
    document, 
    input, 
    io,
    interop, 
    notes
  ]


proc handleCtrlC() {.noconv.} =
  discard

setControlCHook(handleCtrlC)

static:
  writeHeader()
