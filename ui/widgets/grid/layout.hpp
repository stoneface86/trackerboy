// private header file used by PatternPainter and PatternGrid
// contains layout information for the grid

#pragma once

#include <cstdint>
#include <type_traits>

namespace PatternConstants {

enum ColumnType {
    COLUMN_NOTE,

    // high is the upper nibble (bits 4-7)
    // low is the lower nibble (bits 0-3)

    COLUMN_INSTRUMENT_HIGH,
    COLUMN_INSTRUMENT_LOW,

    COLUMN_EFFECT1_TYPE,
    COLUMN_EFFECT1_ARG_HIGH,
    COLUMN_EFFECT1_ARG_LOW,

    COLUMN_EFFECT2_TYPE,
    COLUMN_EFFECT2_ARG_HIGH,
    COLUMN_EFFECT2_ARG_LOW,

    COLUMN_EFFECT3_TYPE,
    COLUMN_EFFECT3_ARG_HIGH,
    COLUMN_EFFECT3_ARG_LOW

};

constexpr int ROWNO_CELLS = 4; // 4 cells for row numbers

constexpr int TRACK_CELLS = 18;
constexpr int TRACK_COLUMNS = 12;

// total columns on the grid (excludes rowno)
constexpr int COLUMNS = TRACK_COLUMNS * 4;

// converts a column index -> cell index
extern uint8_t TRACK_CELL_MAP[TRACK_CELLS];

// converts a cell index -> column index
extern uint8_t TRACK_COLUMN_MAP[TRACK_COLUMNS];

}
