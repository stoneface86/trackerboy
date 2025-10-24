
import
  ./[interop]

import libtrackerboy/notes as lib

header:
  constexpr("Letter", Letter)
  constexpr("NoiseNote", NoiseNote)
  constexpr("ToneNote", ToneNote)
  constexpr("NoteRange", NoteRange)
  constexpr("Octave", Octave)
  constexpr("NoteCut", lib.noteCut)
  constexpr("NoteC", lib.C)
  constexpr("NoteDb", lib.DFlat)
  constexpr("NoteD", lib.D)
  constexpr("NoteEb", lib.EFlat)
  constexpr("NoteE", lib.E)
  constexpr("NoteF", lib.F)
  constexpr("NoteGb", lib.GFlat)
  constexpr("NoteG", lib.G)
  constexpr("NoteAb", lib.AFlat)
  constexpr("NoteA", lib.A)
  constexpr("NoteBb", lib.BFlat)
  constexpr("NoteB", lib.B)

proc bLookupNoiseNote*(note: int): uint8 {.front.} =
  result = lib.lookupNoiseNote(note)

proc bLookupToneNote*(note: int): uint16 {.front.} =
  result = lib.lookupToneNote(note)

