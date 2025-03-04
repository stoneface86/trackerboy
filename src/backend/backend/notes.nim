
import
  ./core,
  ./cgen

import libtrackerboy/notes as lib

static:
  constvar("Letter", Letter)
  constvar("NoiseNote", NoiseNote)
  constvar("ToneNote", ToneNote)
  constvar("NoteRange", NoteRange)
  constvar("Octave", Octave)

proc lookupNoiseNote*(note: int): uint8 {.front.} =
  result = lib.lookupNoiseNote(note)

proc lookupToneNote*(note: int): uint16 {.front.} =
  result = lib.lookupToneNote(note)

