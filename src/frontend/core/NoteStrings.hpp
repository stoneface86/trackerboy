
#pragma once

#include <array>

//
// Namespace contains lookup tables for converting a note index (0-11)
// to a string (ie 0 is "C-", 3 is "D#" or "Eb")
//
namespace NoteStrings {

// type alias for a lookup table of note strings
using NoteTable = std::array<const char *, 12>;

//
// NoteTable with sharp accidentals (Sharps[1] == "C#")
//
extern NoteTable const Sharps;

//
// NoteTable with flat accidentals (Flats[1] == "Db")
//
extern NoteTable const Flats;



}

