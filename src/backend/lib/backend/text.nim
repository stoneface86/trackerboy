import 
  ./[core, interop],
  libtrackerboy/[data, text]

type
  IdString* {.exportc.} = Box[InstrumentString]

proc text*(id: TableId): IdString
  {.front.} =
  result.data = instrumentText(instrumentColumn(id))