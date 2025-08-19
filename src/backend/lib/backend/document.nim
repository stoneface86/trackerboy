
import 
  ./[core, interop],
  libtrackerboy/data,
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
  SongCursor* {.exportc, nonCopyable.} = object
    song: ref Song
    parent: ptr Module

  ModuleCursor* {.exportc, nonCopyable.} = object
    obj: Module

  Document* {.exportc.} = object
    # public
    song*: SongCursor
    `mod`*: ModuleCursor
    # private
    pLock: Lock


  ModuleProperties* {.exportc.} = object
    title*: InfoString
    artist*: InfoString
    copyright*: InfoString
    tickrate*: Tickrate
    revMajor*: uint8
    revMinor*: uint8

static:
  constvar("SystemDmg", systemDmg.uint8)
  constvar("SystemSgb", systemSgb.uint8)
  constvar("SystemCustom", systemCustom.uint8)
  constvar("InfoStringLen", len(InfoString))
  constvar("SpeedLow", rangeSpeed.a.int)
  constvar("SpeedHigh", rangeSpeed.b.int)
  # provide aliases for the ugly identifiers nim generates
  alias("Tickrate", "decltype(ModuleProperties::tickrate)")
  alias("System", "decltype(Tickrate::system)")
  alias("InfoString", "decltype(ModuleProperties::title)")

template module*(d: Document): Module =
  d.`mod`.obj

template `module=`(d: var Document; m: Module) =
  d.`mod`.obj = m

template `@`(s: SongCursor): Song =
  s.song[]

template `@`(m: ModuleCursor): Module =
  m.obj

proc resetCursors(d: var Document) =
  d.song.song = d.module.songs.mget(0)

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
    d.song.song = d.module.songs.mget(songNo)

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

# proc setNameOfLast*(t: var SongListChanges; name: string)
#   {.front, automember.} =
#   proc setName(c: var SongListChange; name: string) =
#     c.name = name
#     c.setName = true
#   t.data[^1].setName(name)

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

