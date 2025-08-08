
import 
  ./[core, interop],
  libtrackerboy/data,
  std/[locks]

type
  BSystem* {.exportc: "System" .} = Wrapper[System]
  BTickrate* {.exportc: "Tickrate".} = Wrapper[Tickrate]
  BInfoString* {.exportc: "InfoString".} = Wrapper[InfoString]

  DocumentPrivate = object
    module: Module
    lock: Lock
    defaultSongName: string
    song: ref Song

  Document* {.exportc.} = object
    p: DocumentPrivate


  ModuleProperties* {.exportc.} = object
    title*: InfoString
    artist*: InfoString
    copyright*: InfoString
    tickrate*: BTickrate
    revMajor*: uint8
    revMinor*: uint8

static:
  constvar("SystemDmg", systemDmg.uint8)
  constvar("SystemSgb", systemSgb.uint8)
  constvar("SystemCustom", systemCustom.uint8)
  constvar("InfoStringLen", len(InfoString))

template module*(d: Document): Module =
  d.p.module

template module*(d: var Document): var Module =
  d.p.module

proc destructor(d: var Document) 
  {.front, autodestructor.} =
  d.p.lock.deinitLock()
  `=destroy`(d)

proc nameFirstSong(d: var Document) =
  d.p.module.songs.mget(0)[].name = d.p.defaultSongName

proc comments*(d: Document): BSlice 
  {.front, automember.} =
  result = slice(d.p.module.comments)

proc setComments*(d: var Document; comments: string)
  {.front, automember.} =
  d.p.module.comments = comments

proc pushNew*(d: var Document)
  {.front, automember.} =
  d.p.lock.acquire()
  d.p.module = initModule()
  d.nameFirstSong()
  d.p.lock.release()

proc newDocument*(defaultSongName: string): ref Document
  {.front.} =
  result = (ref Document)(
    p: DocumentPrivate(
      module: initModule(),
      defaultSongName: defaultSongName
    )
  )
  result[].nameFirstSong()
  result[].p.lock.initLock()

frontRef(ref Document)

proc lock*(d: var Document)
  {.front, automember.} =
  d.p.lock.acquire()

proc unlock*(d: var Document) 
  {.front, automember.} =
  d.p.lock.release()

proc songCount*(d: Document): int 
  {.front, automember.} =
  result = d.p.module.songs.len()

proc songId*(d: Document; index: int): int
  {.front, automember.} =
  result = cast[int](d.p.module.songs.get(index))

proc songName*(d: Document; index: int): BSlice 
  {.front, automember.} =
  let song = d.p.module.songs.get(index)
  result = slice(song[].name)

proc moduleProperties*(d: Document): ModuleProperties
  {.front, automember.} =
  result.title = d.p.module.title
  result.artist = d.p.module.artist
  result.copyright = d.p.module.copyright
  result.tickrate = d.p.module.tickrate
  result.revMajor = uint8(d.p.module.revisionMajor())
  result.revMinor = uint8(d.p.module.revisionMinor())

proc setModuleProperties*(d: var Document; props {.byref.}: ModuleProperties)
  {.front, automember.} =
  d.p.module.title = props.title
  d.p.module.artist = props.artist
  d.p.module.copyright = props.copyright
  d.p.module.tickrate = props.tickrate

proc selectSong*(d: var Document; songNo: int)
  {.front, automember.} =
  if songNo in 0..<d.p.module.songs.len:
    d.p.song = d.p.module.songs.mget(songNo)

type
  SongListChangeKind = enum
    keep
    add
    duplicate
  
  SongListChange = object
    kind: SongListChangeKind
    index: uint8
    setName: bool
    name: string

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

proc setNameOfLast*(t: var SongListChanges; name: string)
  {.front, automember.} =
  proc setName(c: var SongListChange; name: string) =
    c.name = name
    c.setName = true
  t.data[^1].setName(name)

proc setSongList*(d: var Document; changes {.bycref.}: SongListChanges)
  {.front, automember.} =
  var list: seq[ref Song]

  for change in changes.data:
    var song: ref Song
    
    case change.kind
    of keep:
      song = d.p.module.songs.mget(change.index)
    of add:
      song = newSong()
    of duplicate:
      new(song)
      song[] = d.p.module.songs.get(change.index)[]
    
    if change.setName:
      song.name = change.name
    list.add(song)

  d.p.module.songs.data() = list

