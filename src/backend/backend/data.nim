
import 
  ./[core, interop],
  std/[locks]

import libtrackerboy/data as lib

type
  # BInstrument* {. exportc: "Instrument" .} = Box[lib.Instrument]
  # BWaveform* {. exportc: "Waveform" .} = Box[lib.Waveform]
  
  # BModule* {.exportc: "Module".} = Box[lib.Module]
  # BSong* {.exportc: "Song" .} = Box[lib.Song]

  Document* {.exportc.} = object
    module: lib.Module
    lock: Lock

proc `=destroy`*(d: Document) =
  echo "Document destroyed"
  d.lock.deinitLock()

proc newDocument*(): ref Document {.front.} =
  result = (ref Document)(module: lib.initModule())
  result.lock.initLock()

frontRef(ref Document)

proc comments*(d: ref Document): BSlice {.front.} =
  result = slice(d[].module.comments)

proc setComments*(d: ref Document; comments: BSlice) {.front.} =
  assign(d[].module.comments, comments)

proc lock*(d: ref Document) {.front.} =
  d.lock.acquire()

proc unlock*(d: ref Document) {.front.} =
  d.lock.release()