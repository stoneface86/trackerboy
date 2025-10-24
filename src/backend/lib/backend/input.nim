
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
  BNoteBindings* {.exportc, codegenDecl: noteBindingsCodegen.} = object
    map*: array[bindingCount, int32]

  NoteKeymapPrivate = object
    octave: int8
    keycodes: array[bindingCount, int32]
    notes: array[bindingCount, int8]

  BNoteKeymap* {.exportc.} = object
    pd: NoteKeymapPrivate

members(BNoteKeymap):
  constructor:
    proc _(): _ =
      result = BNoteKeymap(
        pd: NoteKeymapPrivate(
          octave: 5 - 2
        )
      )
      @result.notes.fill(int8.low)

  proc keyToNote*(m: _; key: int32): int8 =
    let index = binarySearch(@m.keycodes, key)
    if index == -1:
      result = -1
    else:
      let note = @m.notes[index]
      if note == bindsSpecialNoteCut:
        result = int8(noteCut)
      else:
        result = note + (@m.octave * 12)
        if result.int notin NoteRange.low..NoteRange.high:
          result = -1

  proc octave*(m: _): int8 =
    result = @m.octave + 2

  proc setOctave*(m: var _; octave: int8) =
    if octave.int in Octave.low..Octave.high:
      @m.octave = octave - 2

  proc setBindings*(m: var _; bindings {.bycref.}: BNoteBindings) =
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
      @m.keycodes[i] = pair[0]
      @m.notes[i] = pair[1]
      inc i
  
