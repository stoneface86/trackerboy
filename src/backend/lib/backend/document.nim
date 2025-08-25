
import 
  ./[core, interop],
  libtrackerboy/[data, text],
  std/[locks]

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
  ModuleCursor* {.exportc, nonCopyable.} = object
    obj: Module
  
  SongCursor* {.exportc, nonCopyable.} = object
    `ref`*: ref Song
    parent*: ptr Module

  InstrumentCursor* {.exportc, nonCopyable.} = object
    `ref`: ref Instrument
  
  WaveformCursor* {.exportc, nonCopyable.} = object
    `ref`: ref Waveform

  Document* {.exportc.} = object
    # public
    `mod`*: ModuleCursor
    song*: SongCursor
    inst*: InstrumentCursor
    wave*: WaveformCursor
    # private
    pLock: Lock

  ModuleProperties* {.exportc.} = object
    title*: InfoString
    artist*: InfoString
    copyright*: InfoString
    tickrate*: Tickrate
    revMajor*: uint8
    revMinor*: uint8

  ItemCategory* {.exportc.} = enum
    catSong
    catInstrument
    catWaveform

  ItemName* {.exportc.} = object
    id*: uint8
    value*: BSlice


static:
  constvar("InfoStringLen", len(InfoString))
  constvar("SpeedLow", rangeSpeed.a.int)
  constvar("SpeedHigh", rangeSpeed.b.int)
  # provide aliases for the ugly identifiers nim generates
  alias("Tickrate", "decltype(ModuleProperties::tickrate)")
  alias("InfoString", "decltype(ModuleProperties::title)")
  # expose enums to the front-end as constexpr variables
  frontEnum(ChannelId)
  frontEnum(System)
  frontEnum(SequenceKind)
  frontEnum(ItemCategory)

template module*(d: Document): Module =
  d.`mod`.obj

template `module=`(d: var Document; m: Module) =
  d.`mod`.obj = m

template `@`(c: SongCursor | InstrumentCursor | WaveformCursor): auto =
  c.`ref`[]

template `@`(m: ModuleCursor): Module =
  m.obj

proc resetCursors(d: var Document) =
  d.song.`ref` = d.module.songs.mget(0)

proc destructor(d: var Document) 
  {.front, autodestructor.} =
  d.pLock.deinitLock()
  `=destroy`(d)

# Document methods

proc pushNew*(d: var Document)
  {.front, automember.} =
  d.module = initModule()
  d.resetCursors()

proc newDocument*(): ref Document
  {.front.} =
  result = (ref Document)(
    `mod`: ModuleCursor(
      obj: initModule()
    )
  )
  result[].song.parent = addr(result[].`mod`.obj)
  resetCursors(result[])
  result[].pLock.initLock()

frontRef(ref Document)

proc lock*(d: var Document)
  {.front, automember.} =
  d.pLock.acquire()

proc unlock*(d: var Document) 
  {.front, automember.} =
  d.pLock.release()

proc selectSong*(d: var Document; songNo: int)
  {.front, automember.} =
  if songNo in 0 ..< d.module.songs.len:
    d.song.`ref` = d.module.songs.mget(songNo)

proc getOrNil(t: var SomeTable; id: int): auto =
  if id in 0 .. high(TableId).int:
    result = t[TableId(id)]

proc selectInstrument*(d: var Document; id: int) =
  d.inst.`ref` = getOrNil(d.module.instruments, id)

proc selectWaveform*(d: var Document; id: int) =
  d.wave.`ref` = getOrNil(d.module.waveforms, id)

# ModuleCursor methods

proc comments*(m: ModuleCursor): BSlice 
  {.front, automember.} =
  result = slice(@m.comments)

proc setComments*(m: var ModuleCursor; comments: string)
  {.front, automember.} =
  @m.comments = comments

proc songCount*(m: ModuleCursor): int 
  {.front, automember.} =
  result = @m.songs.len()

proc songId*(m: ModuleCursor; index: int): int
  {.front, automember.} =
  result = cast[int](@m.songs.get(index))

proc songName*(m: ModuleCursor; index: int): BSlice 
  {.front, automember.} =
  let song = @m.songs.get(index)
  result = slice(song[].name)

proc setSongName*(m: var ModuleCursor; index: int; name: string)
  {.front, automember.} =
  @m.songs.mget(index)[].name = name

proc moduleProperties*(m: ModuleCursor): ModuleProperties
  {.front, automember.} =
  result.title = @m.title
  result.artist = @m.artist
  result.copyright = @m.copyright
  result.tickrate = @m.tickrate
  result.revMajor = uint8(@m.revisionMajor())
  result.revMinor = uint8(@m.revisionMinor())

proc setModuleProperties*(m: var ModuleCursor; props {.byref.}: ModuleProperties)
  {.front, automember.} =
  @m.title = props.title
  @m.artist = props.artist
  @m.copyright = props.copyright
  @m.tickrate = props.tickrate

# item management

proc itemCount*(m: ModuleCursor; cat: ItemCategory): int
  {.front, automember.} =
  case cat
  of catSong: @m.songs.len()
  of catInstrument: @m.instruments.len()
  of catWaveform: @m.waveforms.len()

proc itemName*(m: ModuleCursor; cat: ItemCategory; id: uint8): ItemName
  {.front, automember.} =

  proc itemNameInTable(t: SomeTable; startingId: uint8): ItemName =
    var id = TableId(startingId)
    while id < high(TableId):
      if id in t:
        result.id = id
        result.value = slice(t[id][].name)
        break
      inc id

  case cat
  of catSong:
    result.id = id
    result.value = slice(@m.songs.get(id)[].name)
  of catInstrument:
    result = itemNameInTable(@m.instruments, id)
  of catWaveform:
    result = itemNameInTable(@m.waveforms, id)
  

proc itemSetName*(m: var ModuleCursor; cat: ItemCategory; id: uint8;
                  name: string)
  {.front, automember.} =
  proc setName(t: var SomeTable; id: uint8; name: string) =
    t[TableId(id)].name = name
  
  case cat
  of catSong:
    @m.songs.mget(id).name = name
  of catInstrument:
    setName(@m.instruments, id, name)
  of catWaveform:
    setName(@m.waveforms, id, name)
  

# SongCursor methods

proc rowsPerBeat*(s: SongCursor): int
  {.front, automember.} =
  result = @s.rowsPerBeat

proc rowsPerMeasure*(s: SongCursor): int
  {.front, automember.} =
  result = @s.rowsPerMeasure

proc speed*(s: SongCursor): Speed
  {.front, automember.} =
  result = @s.speed

proc speedFloat*(s: SongCursor): float32
  {.front, automember.} =
  result = toFloat(@s.speed)

proc tempo*(s: SongCursor): float32
  {.front, automember.} =
  result = @s.tempo(@s.effectiveTickrate(s.parent[].tickrate).hertz)

proc trackLen*(s: SongCursor): int
  {.front, automember.} =
  result = @s.trackLen

proc hasTickrate*(s: SongCursor): bool
  {.front, automember.} =
  result = @s.tickrate.isSome()

proc tickrate*(s: SongCursor): Tickrate
  {.front, automember.} =
  result = @s.tickrate.get(defaultTickrate)

proc setRowsPerBeat*(s: var SongCursor; rowsPerBeat: int)
  {.front, automember.} =
  @s.rowsPerBeat = ByteIndex(rowsPerBeat)

proc setRowsPerMeasure*(s: var SongCursor; rowsPerMeasure: int)
  {.front, automember.} =
  @s.rowsPerMeasure = ByteIndex(rowsPerMeasure)

proc setSpeed*(s: var SongCursor; speed: int)
  {.front, automember.} =
  @s.speed = Speed(speed)

proc setTrackLen*(s: var SongCursor; len: int)
  {.front, automember.} =
  @s.trackLen = PositiveByte(len)

proc tickrateEqual(s: SongCursor; rate: Tickrate): bool
  {.front, automember.} =
  result = @s.tickrate.isSome() and @s.tickrate.unsafeGet() == rate

proc setTickrate*(s: var SongCursor; rate: Tickrate)
  {.front, automember.} =
  @s.tickrate = some(rate)

proc clearTickrate*(s: var SongCursor)
  {.front, automember.} =
  @s.tickrate = none(Tickrate)

type
  SongListChangeKind = enum
    keep
    add
    duplicate
  
  SongListChange = object
    kind: SongListChangeKind
    index: uint8

  SongListChanges* {.exportc.} = object
    data: seq[SongListChange]

proc initSongListChanges*(): SongListChanges
  {.front.} =
  result.data = newSeq[SongListChange]()

proc destructor*(t: var SongListChanges)
  {.front, autodestructor.} =
  `=destroy`(t)

proc keepOriginal*(t: var SongListChanges; index: uint8)
  {.front, automember.} =
  t.data.add(SongListChange(kind: keep, index: index))

proc addNew*(t: var SongListChanges)
  {.front, automember.} =
  t.data.add(SongListChange(kind: add))

proc duplicate*(t: var SongListChanges; index: uint8)
  {.front, automember.} =
  t.data.add(SongListChange(kind: duplicate, index: index))

proc setSongList*(d: var Document; changes {.bycref.}: SongListChanges)
  {.front, automember.} =
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

# InstrumentCursor methods

proc channel*(i: InstrumentCursor): ChannelId
  {.front, automember.} =
  result = @i.channel

proc setChannel*(i: var InstrumentCursor; ch: ChannelId)
  {.front, automember.} =
  @i.channel = ch

proc sample*(c: InstrumentCursor; sk: SequenceKind; i: int): int8
  {.front, automember.} =
  result = cast[int8](@c.sequences[sk][i])

proc setSample*(c: var InstrumentCursor; sk: SequenceKind; i: int; sample: int8)
  {.front, automember.} =
  @c.sequences[sk][i] = cast[uint8](sample)

proc sequenceLen*(c: InstrumentCursor; sk: SequenceKind): int
  {.front, automember.} =
  result = @c.sequences[sk].len()

proc setSequenceLen*(c: var InstrumentCursor; sk: SequenceKind; len: int)
  {.front, automember.} =
  @c.sequences[sk].setLen(len)

proc sequenceAsText*(c: InstrumentCursor; sk: SequenceKind): string
  {.front, automember.} =
  result = sequenceText(@c.sequences[sk])

proc setSequence*(c: var InstrumentCursor; sk: SequenceKind; text: string;
                  minVal: int8; maxVal: int8;
                  ): bool
  {.front, automember.} =
  let parsed = parseSequence(text, minVal, maxVal)
  result = parsed.isSome()
  if result:
    @c.sequences[sk] = parsed.get()

# WaveformCursor methods

proc sample*(w: WaveformCursor; i: int): int8
  {.front, automember.} =
  let pair = @w.data[i]
  if (uint(i) and 1) == 0:
    result = int8(pair shr 4)
  else:
    result = int8(pair and 0xF)

proc setSample*(w: var WaveformCursor; i: int; sample: int8)
  {.front, automember.} =
  let pair = addr(@w.data[i div 2])
  if (uint(i) and 1) == 0:
    pair[] = (pair[] and 0x0F) or (uint8(sample) shl 4)
  else:
    pair[] = (pair[] and 0xF0) or uint8(sample)

proc asText*(w: WaveformCursor; outText: var WaveDataString)
  {.front, automember.} =
  outText = waveText(@w.data)

proc setFromText*(w: var WaveformCursor; text: WaveDataString)
  {.front, automember.} =
  let parsed = parseWave(text)
  if parsed.isSome():
    @w.data = parsed.get()

