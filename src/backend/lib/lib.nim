
{. push warning[UnusedImport]: off .}

import
  ./backend/[
    core, 
    document, 
    input, 
    io,
    interop, 
    notes,
    text
  ]


proc handleCtrlC() {.noconv.} =
  discard

setControlCHook(handleCtrlC)

writeHeader()
