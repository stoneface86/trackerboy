{.used.}

import
  std/unittest,
  backend/input,
  libtrackerboy/notes

template `@`(k: ref NoteKeymap): ptr NoteKeymap =
  cast[ptr NoteKeymap](k)


suite "NoteKeymap":

  test "default returns -1 on keyToNote":
    let km = newNoteKeymap()
    check:
      km[].keyToNote(0) == -1
      km[].keyToNote(100) == -1
  
  test "default octave is 5":
    let km = newNoteKeymap()
    check km[].octave() == 5

  test "setOctave":
    var km = newNoteKeymap()
    let octave = km[].octave()
    @km.setOctave(0)
    check km[].octave() == octave
    @km.setOctave(2)
    check km[].octave() == 2
    @km.setOctave(13)
    check km[].octave() == 2

  test "setBindings 1":
    var 
      km = newNoteKeymap()
      bindings: NoteBindings
    const
      k1 = 0x1000
      k2 = 0x1001
      k3 = 0x2000
      k4 = 0x3000
    # bind C-0 to 0x1000
    bindings.map[0] = k1
    # bind C#0 to 0x1001
    bindings.map[1] = k2
    # bind C-1 to 0x2000
    bindings.map[bindsUpperRow.a] = k3
    # bind note cut to 0x30000
    bindings.map[bindsSpecialNoteCut] = k4
    @km.setBindings(bindings)
    @km.setOctave(2)

    check:
      km[].keyToNote(k1) == 0
      km[].keyToNote(k2) == 1
      km[].keyToNote(k3) == bindsUpperRow.a
      km[].keyToNote(k4) == noteCut.int8
      km[].keyToNote(0xFFF) == -1
    
    @km.setOctave(8)
    const offset = (8 - 2) * 12
    check:
      km[].keyToNote(k1) == 0 + offset
      km[].keyToNote(k2) == 1 + offset
      km[].keyToNote(k3) == -1
      km[].keyToNote(k4) == noteCut.int8
      km[].keyToNote(0xFFF) == -1

