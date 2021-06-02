// private header file used by PatternPainter and PatternGrid
// contains layout information for the grid

#pragma once

#include <cstdint>
#include <type_traits>

namespace PatternConstants {

constexpr int ROWNO_CELLS = 4; // 4 cells for row numbers

constexpr int TRACK_CELLS = 20;
constexpr int TRACK_COLUMNS = 12;
constexpr int TRACK_DATA_COLUMNS = 5; // note, instrument + 3 effects

// total columns on the grid (excludes rowno)
constexpr int COLUMNS = TRACK_COLUMNS * 4;

// converts a column index -> cell index
extern uint8_t TRACK_CELL_MAP[TRACK_CELLS];

// converts a cell index -> column index
extern uint8_t TRACK_COLUMN_MAP[TRACK_COLUMNS];

extern uint8_t TRACK_CELL_TO_DATA_COLUMN[TRACK_CELLS];

}
