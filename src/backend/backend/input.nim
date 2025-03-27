
import
  ./[interop],
  libtrackerboy/notes,
  std/[algorithm]

const
  bindsPerRow = 24
  bindsLowerRow = 0..(bindsPerRow-1)
  bindsUpperRow = (bindsLowerRow.a)..(bindsLowerRow.b + bindsPerRow)
  bindsSpecialNoteCut = bindsUpperRow.b + 1
  bindingCount = bindsSpecialNoteCut + 1


type
  NoteBinding = object
    keycode: int32
    note: int8

  NoteBindings* {.exportc.} = object
    map*: array[bindingCount, int32]

  NoteKeymap* {.exportc.} = object
    octave: int8
    bindings: array[bindingCount, NoteBinding]

func `<`(x, y: NoteBinding; ): bool =
  result = x.keycode < y.keycode

proc newNoteKeymap*(): ref NoteKeymap {.front.} =
  result = (ref NoteKeymap)(octave: 5)

frontRef(ref NoteKeymap)

proc keyToNote*(m: ref NoteKeymap; key: int32): int8 {.front.} =
  let 
    filter = NoteBinding(keycode: key)
    index = binarySearch(m.bindings, filter)
  if index == -1:
    result = -1
  else:
    let note = m.bindings[index].note
    if note == bindsSpecialNoteCut:
      result = int8(noteCut)
    else:
      result = note + (m.octave * 12)
      if result.int notin NoteRange.low..NoteRange.high:
        result = -1

proc setBindings*(m: ref NoteKeymap; bindings: NoteBindings) {.front.} =
  var i = 0
  for note, keycode in pairs(bindings.map):
    m.bindings[i] = NoteBinding(keycode: keycode, note: int8(note))
  sort(m.bindings)

static:
  constvar("NoteBindingsLen", bindingCount)