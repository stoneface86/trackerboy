
import 
  ./[core, interop],
  std/[locks]

import libtrackerboy/data as lib

type
  # BInstrument* {. exportc: "Instrument" .} = Box[lib.Instrument]
  # BWaveform* {. exportc: "Waveform" .} = Box[lib.Waveform]
  
  # BModule* {.exportc: "Module".} = Box[lib.Module]
  # BSong* {.exportc: "Song" .} = Box[lib.Song]

  DocumentPrivate = object
    module: lib.Module
    lock: Lock
    defaultSongName: string

  Document* {.exportc.} = object
    p: DocumentPrivate

proc destructor(d: ptr Document) 
  {.member: "~Document()", used.} =
  d.p.lock.deinitLock()
  `=destroy`(d[])

proc nameFirstSong(d: var Document) =
  d.p.module.songs.mget(0)[].name = d.p.defaultSongName

proc comments*(d: Document): BSlice 
  {.front, member: "$1() const -> '0".} =
  result = slice(d.p.module.comments)

proc setComments*(d: ptr Document; comments: string) {.front, member.} =
  d[].p.module.comments = comments

proc pushNew*(d: ptr Document) {.front, member.} =
  d[].p.lock.acquire()
  d[].p.module = initModule()
  d[].nameFirstSong()
  d[].p.lock.release()

proc newDocument*(defaultSongName: string): ref Document {.front.} =
  result = (ref Document)(
    p: DocumentPrivate(
      module: lib.initModule(),
      defaultSongName: defaultSongName
    )
  )
  result[].nameFirstSong()
  result[].p.lock.initLock()

frontRef(ref Document)

proc lock*(d: ptr Document)
  {.front, member.} =
  d[].p.lock.acquire()

proc unlock*(d: ptr Document) 
  {.front, member.} =
  d[].p.lock.release()

proc songCount*(d: Document): int 
  {.front, member: "$1() const -> '0".} =
  result = d.p.module.songs.len()

proc songName*(d: Document; index: int): BSlice 
  {.front, member: "$1('2 #2) const -> '0".} =
  let song = d.p.module.songs.get(index)
  result = slice(song[].name)

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

proc destructor*(t: ptr SongListChanges)
  {.front, member: "~SongListChanges()".} =
  `=destroy`(t[])

proc keepOriginal*(t: ptr SongListChanges; index: uint8)
  {.front, member.} =
  t[].data.add(SongListChange(kind: keep, index: index))

proc addNew*(t: ptr SongListChanges)
  {.front, member.} =
  t[].data.add(SongListChange(kind: add))

proc duplicate*(t: ptr SongListChanges; index: uint8)
  {.front, member.} =
  t[].data.add(SongListChange(kind: duplicate, index: index))

proc setNameOfLast*(t: ptr SongListChanges; name: string)
  {.front, member.} =
  proc setName(c: var SongListChange; name: string) =
    c.name = name
    c.setName = true
  t[].data[^1].setName(name)

proc setSongList*(d: ptr Document; changes: SongListChanges)
  {.front, member: "$1('2 const& #2)".} =
  var list: seq[ref lib.Song]

  for change in changes.data:
    var song: ref lib.Song
    
    case change.kind
    of keep:
      song = d[].p.module.songs.mget(change.index)
    of add:
      song = lib.newSong()
    of duplicate:
      new(song)
      song[] = d[].p.module.songs.get(change.index)[]
    
    if change.setName:
      song.name = change.name
    list.add(song)

  d[].p.module.songs.data() = list