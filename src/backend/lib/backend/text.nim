import 
  ./[core, interop],
  libtrackerboy/[data, text]

type
  BIdString* {.exportc.} = Box[InstrumentString]

proc bText*(id: TableId): BIdString
  {.front.} =
  result.data = instrumentText(instrumentColumn(id))
