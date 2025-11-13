
import 
  ./[core, interop],
  libtrackerboy/[data, text],
  std/[locks, sugar]

const nonCopyableDecl = """
struct $1 {
private:
  $1& operator=($1 const& a) = delete;
public:
$2
};
"""
{.pragma: nonCopyable, codegenDecl: nonCopyableDecl.}

type
  BModuleCursor* {.exportc, nonCopyable.} = object
    `ref`: ref Module
  
  BSongCursor* {.exportc, nonCopyable.} = object
    `ref`*: ref Song
    parent*: ref Module

  BInstrumentCursor* {.exportc, nonCopyable.} = object
    `ref`: ref Instrument
  
  BWaveformCursor* {.exportc, nonCopyable.} = object
    `ref`: ref Waveform

  BDocumentPrivate = object
    lock: Lock

  BDocument* {.exportc.} = object
    # public
    `mod`*: BModuleCursor
    song*: BSongCursor
    inst*: BInstrumentCursor
    wave*: BWaveformCursor
    # private
    pd: BDocumentPrivate

  BModuleProperties* {.exportc.} = object
    title*: InfoString
    artist*: InfoString
    copyright*: InfoString
    tickrate*: Tickrate
    revMajor*: uint8
    revMinor*: uint8

  BItemCategory* {.exportc.} = enum
    catSong
    catInstrument
    catWaveform

  BItemName* {.exportc.} = object
    id*: uint8
    value*: BSlice

  ItemizerImpl = object
    count: proc(m: Module): int {.nimcall, raises: [].}
    name: proc(m: Module; id: uint8): BItemName {.nimcall, raises: [].}
    setName: proc(m: var Module; id: uint8; name: string) {.nimcall, raises: [].}

  BItemizer* {.exportc.} = object
    impl: ptr ItemizerImpl

  TableModelImpl = object
    add: proc(m: var Module; id: TableId) {.nimcall, raises: [].}
    addNext: proc(m: var Module): TableId {.nimcall, raises: [].}
    remove: proc(m: var Module; id: TableId) {.nimcall, raises: [].}
    duplicate: proc(m: var Module; id: TableId): TableId {.nimcall, raises: [].}

  TableIdMap = object
    ## Maps a TableId to an integer index in a list.
    ## 
    map: array[TableId, uint8]

  BTableModel* {.exportc.} = object
    ## API object used by the BTableModel class. Allows the model to add, remove
    ## duplicate new items and also maintains a table id to list id mapping.
    ## 
    impl: ptr TableModelImpl
    showEmpty: bool
    idMap: TableIdMap

  HasImpl = concept a
    a.impl is ptr

header:
  constexpr("InfoStringLen", len(InfoString))
  constexpr("TableCap", 64)
  constexpr("SongCap", 256)
  constexpr("SpeedLow", rangeSpeed.a.int)
  constexpr("SpeedHigh", rangeSpeed.b.int)
  # provide aliases for the ugly identifiers nim generates
  alias("BTickrate", "decltype(BModuleProperties::tickrate)")
  alias("BInfoString", "decltype(BModuleProperties::title)")
  # expose enums to the front-end as constexpr variables
  addEnum(ChannelId, true)
  addEnum(System, true)
  addEnum(SequenceKind, true)
  addEnum(BItemCategory)


const
  TableIdMapNone = high(TableId) + 1

template module*(d: BDocument): Module =
  d.`mod`.`ref`[]

template `module=`*(d: var BDocument; m: Module) =
  d.`mod`.`ref`[] = m

template `@`(c: BModuleCursor | BSongCursor | BInstrumentCursor | BWaveformCursor): auto =
  c.`ref`[]

template `@`[T: HasImpl](x: T): auto =
  x.impl[]

proc resetCursors(d: var BDocument) =
  d.song.`ref` = d.module.songs.mget(0)

proc getOrNil(t: var SomeTable; id: int): auto =
  if id in 0 .. high(TableId).int:
    result = t[TableId(id)]

members(BDocument):
  constructor:
    proc _(): _ =
      result.`mod`.`ref` = newModule()
      result.song.parent = result.`mod`.`ref`
      result.resetCursors()
      @result.lock.initLock()
  
  destructor(d):
    @d.lock.deinitLock()
    `=destroy`(d)

  proc pushNew(d: var _) =
    d.module = initModule()
    d.resetCursors()

  proc lock*(d: var _) =
    @d.lock.acquire()

  proc unlock*(d: var _) =
    @d.lock.release()

  proc selectSong*(d: var _; songNo: int) =
    if songNo in 0 ..< d.module.songs.len:
      d.song.`ref` = d.module.songs.mget(songNo)
  
  proc selectInstrument*(d: var _; id: int) =
    d.inst.`ref` = getOrNil(d.module.instruments, id)

  proc selectWaveform*(d: var _; id: int) =
    d.wave.`ref` = getOrNil(d.module.waveforms, id)

members(BModuleCursor):

  proc comments*(m: _): BSlice =
    result = slice(@m.comments)

  proc setComments*(m: var _; comments: string) =
    @m.comments = comments

  proc songCount*(m: _): int =
    result = @m.songs.len()

  proc songId*(m: _; index: int): int =
    result = cast[int](@m.songs.get(index))

  proc songName*(m: _; index: int): BSlice =
    let song = @m.songs.get(index)
    result = slice(song[].name)

  proc setSongName*(m: var _; index: int; name: string) =
    @m.songs.mget(index)[].name = name

  proc moduleProperties*(m: _; outProps: var BModuleProperties) =
    outProps.title = @m.title
    outProps.artist = @m.artist
    outProps.copyright = @m.copyright
    outProps.tickrate = @m.tickrate
    outProps.revMajor = uint8(@m.revisionMajor())
    outProps.revMinor = uint8(@m.revisionMinor())

  proc setModuleProperties*(m: var _; props {.bycref.}: BModuleProperties) =
    @m.title = props.title
    @m.artist = props.artist
    @m.copyright = props.copyright
    @m.tickrate = props.tickrate

# item management
template getTable(m: Module, T: typedesc[SomeData]): auto =
  when T is Instrument:
    m.instruments
  else:
    m.waveforms

proc makeItemizer(T: typedesc[SomeData]): ItemizerImpl {.compileTime.} =
  result.count = proc(m: Module): int =
    result = getTable(m, T).len()
  
  result.name = proc(m: Module; id: uint8): BItemName =
    proc getNearest(t: SomeTable; startingId: uint8): BItemName =
      var id = TableId(startingId)
      while id < high(TableId):
        if id in t:
          result.id = id
          result.value = slice(t[id][].name)
          break
        inc id
    result = getNearest(getTable(m, T), id)

  result.setName = proc(m: var Module; id: uint8; name: string) =
    getTable(m, T)[TableId(id)].name = name

const
  SongItemizer = ItemizerImpl(
    count: proc(m: Module): int =
      result = m.songs.len()
    , name: proc(m: Module; id: uint8): BItemName =
      result.id = id
      result.value = slice(m.songs.get(id)[].name)
    , setName: proc(m: var Module; id: uint8; name: string) =
      m.songs.mget(id).name = name
  )
  InstrumentItemizer = makeItemizer(Instrument)
  WaveformItemizer = makeItemizer(Waveform)

members(BItemizer):
  constructor:
    proc _(cat: BItemCategory): _ =
      result.impl = case cat
      of catSong: addr(SongItemizer)
      of catInstrument: addr(InstrumentItemizer)
      of catWaveform: addr(WaveformItemizer)

  proc count*(i: _; m {.bycref.}: BModuleCursor): int =
    result = @i.count(@m)

  proc name*(i: _; m {.bycref.}: BModuleCursor; id: uint8): BItemName =
    result = @i.name(@m, id)

  proc setName*(i: var _; m: var BModuleCursor; id: uint8; name: string) =
    @i.setName(@m, id, name)

# BTableModel

proc makeTableModel(T: typedesc[SomeData]): TableModelImpl =
  result.add = (m: var Module, id: TableId) => getTable(m, T).add(id)
  result.addNext = (m: var Module) => getTable(m, T).add()
  result.remove = (m: var Module, id: TableId) => getTable(m, T).remove(id)
  result.duplicate = (m: var Module, id: TableId) => getTable(m, T).duplicate(id)

const
  InstrumentTableModel = makeTableModel(Instrument)
  WaveformTableModel = makeTableModel(Waveform)

proc put(m: var TableIdMap; id: TableId) =
  # find the previous index
  var index = 0.uint8
  for i in countDown(id, TableId.low):
    let val = m.map[i]
    if val > 0:
      index = val
      break
  m.map[id] = index + 1
  for i in (id + 1)..TableId.high:
    if m.map[i] > 0:
      inc m.map[i]

proc del(m: var TableIdMap; id: TableId) =
  m.map[id] = 0
  for i in (id+1)..TableId.high:
    if m.map[i] > 0:
      dec m.map[i]

members(BTableModel):
  constructor:
    proc _(cat: BItemCategory): _ =
      result.impl = case cat
      of catSong: nil
      of catInstrument: addr(InstrumentTableModel)
      of catWaveform: addr(WaveformTableModel)

  proc listIndex*(t: _; id: TableId): int =
    result = int(t.idMap.map[id]) - 1

  proc reset*(t: var _) =
    reset(t.idMap)

  proc add*(t: var _; m: var BModuleCursor; id: int): TableId =
    if id == -1 or t.idMap.map[TableId(id)] != 0:
      result = @t.addNext(@m)
    else:
      result = TableId(id)
      @t.add(@m, result)
    t.idMap.put(result)

  proc remove*(t: var _; m: var BModuleCursor; id: TableId) =
    @t.remove(@m, id)
    t.idMap.del(id)

  proc duplicate*(t: var _; m: var BModuleCursor; id: TableId): TableId =
    result = @t.duplicate(@m, id)
    t.idMap.put(result)

  proc assignId*(t: var _; id: uint8; index: uint8; ) =
    t.idMap.map[id] = index + 1

# BSongCursor methods

members(BSongCursor):

  proc rowsPerBeat*(s: _): int =
    result = @s.rowsPerBeat

  proc rowsPerMeasure*(s: _): int =
    result = @s.rowsPerMeasure

  proc speed*(s: _): Speed =
    result = @s.speed

  proc speedFloat*(s: _): float32 =
    result = toFloat(@s.speed)

  proc tempo*(s: _): float32 =
    result = @s.tempo(@s.effectiveTickrate(s.parent[].tickrate).hertz)

  proc trackLen*(s: _): int =
    result = @s.trackLen

  proc hasTickrate*(s: _): bool =
    result = @s.tickrate.isSome()

  proc tickrate*(s: _): Tickrate =
    result = @s.tickrate.get(defaultTickrate)

  proc setRowsPerBeat*(s: var _; rowsPerBeat: int) =
    @s.rowsPerBeat = ByteIndex(rowsPerBeat)

  proc setRowsPerMeasure*(s: var _; rowsPerMeasure: int) =
    @s.rowsPerMeasure = ByteIndex(rowsPerMeasure)

  proc setSpeed*(s: var _; speed: int) =
    @s.speed = Speed(speed)

  proc setTrackLen*(s: var _; len: int) =
    @s.trackLen = PositiveByte(len)

  proc tickrateEqual(s: _; rate: Tickrate): bool =
    result = @s.tickrate.isSome() and @s.tickrate.unsafeGet() == rate

  proc setTickrate*(s: var _; rate: Tickrate) =
    @s.tickrate = some(rate)

  proc clearTickrate*(s: var _) =
    @s.tickrate = none(Tickrate)

type
  SongListChangeKind = enum
    keep
    add
    duplicate
  
  SongListChange = object
    kind: SongListChangeKind
    index: uint8

  BSongListChanges* {.exportc.} = object
    data: seq[SongListChange]

members(BSongListChanges):
  constructor:
    proc _(): _ =
      result.data = newSeq[SongListChange]()
  destructor

  proc keepOriginal*(t: var _; index: uint8) =
    t.data.add(SongListChange(kind: keep, index: index))

  proc addNew*(t: var _) =
    t.data.add(SongListChange(kind: add))

  proc duplicate*(t: var _; index: uint8) =
    t.data.add(SongListChange(kind: duplicate, index: index))

members(BDocument):
  proc setSongList*(d: var _; changes {.bycref.}: BSongListChanges) =
    var list: seq[ref Song]

    for change in changes.data:
      var song: ref Song
      
      case change.kind
      of keep:
        song = d.module.songs.mget(change.index)
      of add:
        song = newSong()
      of duplicate:
        new(song)
        song[] = d.module.songs.get(change.index)[]
      
      # if change.setName:
      #   song.name = change.name
      list.add(song)

    d.module.songs.data() = list

# BInstrumentCursor methods

members(BInstrumentCursor):

  proc channel*(i: _): ChannelId =
    result = @i.channel

  proc setChannel*(i: var _; ch: ChannelId) =
    @i.channel = ch

  proc sample*(c: _; sk: SequenceKind; i: int): int8 = 
    result = cast[int8](@c.sequences[sk][i])

  proc setSample*(c: var _; sk: SequenceKind; i: int; sample: int8) = 
    @c.sequences[sk][i] = cast[uint8](sample)

  proc sequenceLen*(c: _; sk: SequenceKind): int = 
    result = @c.sequences[sk].len()

  proc setSequenceLen*(c: var _; sk: SequenceKind; len: int) =
    @c.sequences[sk].setLen(len)

  proc sequenceAsText*(c: _; sk: SequenceKind): string =
    result = sequenceText(@c.sequences[sk])

  proc setSequence*(c: var _; sk: SequenceKind; text: string;
                    minVal: int8; maxVal: int8;
                    ): bool =
    let parsed = parseSequence(text, minVal, maxVal)
    result = parsed.isSome()
    if result:
      @c.sequences[sk] = parsed.get()

# BWaveformCursor methods

members(BWaveformCursor):
  proc sample*(w: _; i: int): int8 =
    let pair = @w.data[i]
    if (uint(i) and 1) == 0:
      result = int8(pair shr 4)
    else:
      result = int8(pair and 0xF)

  proc setSample*(w: var _; i: int; sample: int8) =
    let pair = addr(@w.data[i div 2])
    if (uint(i) and 1) == 0:
      pair[] = (pair[] and 0x0F) or (uint8(sample) shl 4)
    else:
      pair[] = (pair[] and 0xF0) or uint8(sample)

  proc asText*(w: _; outText: var WaveDataString) =
    outText = waveText(@w.data)

  proc setFromText*(w: var _; text: WaveDataString) =
    let parsed = parseWave(text)
    if parsed.isSome():
      @w.data = parsed.get()
