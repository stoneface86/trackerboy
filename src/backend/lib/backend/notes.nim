
import
  ./[interop]

import libtrackerboy/notes as lib

static:
  constvar("Letter", Letter)
  constvar("NoiseNote", NoiseNote)
  constvar("ToneNote", ToneNote)
  constvar("NoteRange", NoteRange)
  constvar("Octave", Octave)
  constvar("NoteCut", lib.noteCut)
  constvar("NoteC", lib.C)
  constvar("NoteDb", lib.DFlat)
  constvar("NoteD", lib.D)
  constvar("NoteEb", lib.EFlat)
  constvar("NoteE", lib.E)
  constvar("NoteF", lib.F)
  constvar("NoteGb", lib.GFlat)
  constvar("NoteG", lib.G)
  constvar("NoteAb", lib.AFlat)
  constvar("NoteA", lib.A)
  constvar("NoteBb", lib.BFlat)
  constvar("NoteB", lib.B)

proc lookupNoiseNote*(note: int): uint8 {.front.} =
  result = lib.lookupNoiseNote(note)

proc lookupToneNote*(note: int): uint16 {.front.} =
  result = lib.lookupToneNote(note)

