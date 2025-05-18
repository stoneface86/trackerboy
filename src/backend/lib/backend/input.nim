
import
  ./[interop],
  libtrackerboy/notes,
  std/[algorithm, strformat]

const
  bindsPerRow* = 24
  bindsLowerRow* = 0..(bindsPerRow-1)
  bindsUpperRow* = (bindsLowerRow.b+1)..(bindsLowerRow.b + bindsPerRow)
  bindsSpecialNoteCut* = bindsUpperRow.b + 1
  bindingCount* = bindsSpecialNoteCut + 1

  noteBindingsCodegen = &"""
struct $1 {{
  static constexpr auto lowerRow = {bindsLowerRow.a};
  static constexpr auto upperRow = {bindsUpperRow.a};
  static constexpr auto noteCut = {bindsSpecialNoteCut};
  static constexpr auto len = {bindingCount};
$2
}};
"""

type
  NoteBindings* {.exportc, codegenDecl: noteBindingsCodegen.} = object
    map*: array[bindingCount, int32]

  NoteKeymapPrivate = object
    octave: int8
    keycodes: array[bindingCount, int32]
    notes: array[bindingCount, int8]

  NoteKeymap* {.exportc.} = object
    p: NoteKeymapPrivate

proc newNoteKeymap*(): ref NoteKeymap {.front.} =
  result = (ref NoteKeymap)(p: NoteKeymapPrivate(octave: 5 - 2))
  result[].p.notes.fill(int8.low)

frontRef(ref NoteKeymap)

proc keyToNote*(m: NoteKeymap; key: int32): int8 
  {.front, automember.} =
  let index = binarySearch(m.p.keycodes, key)
  if index == -1:
    result = -1
  else:
    let note = m.p.notes[index]
    if note == bindsSpecialNoteCut:
      result = int8(noteCut)
    else:
      result = note + (m.p.octave * 12)
      if result.int notin NoteRange.low..NoteRange.high:
        result = -1

proc octave*(m: NoteKeymap): int8 
  {.front, automember.} =
  result = m.p.octave + 2

proc setOctave*(m: var NoteKeymap; octave: int8)
  {.front, automember.} =
  if octave.int in Octave.low..Octave.high:
    m.p.octave = octave - 2

proc setBindings*(m: var NoteKeymap; bindings {.bycref.}: NoteBindings)
  {.front, automember.} =
  var 
    i = 0
    buffer: array[bindingCount, (int32, int8)]
  for note, keycode in pairs(bindings.map):
    buffer[i] = (keycode, int8(note))
    inc i
  buffer.sort do (x, y: (int32, int8);) -> int:
    result = cmp(x[0], y[0])
  i = 0
  for pair in buffer:
    m.p.keycodes[i] = pair[0]
    m.p.notes[i] = pair[1]
    inc i
 