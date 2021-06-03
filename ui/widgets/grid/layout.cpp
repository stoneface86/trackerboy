
#include "widgets/grid/layout.hpp"

#include "core/PatternCursor.hpp"

namespace PatternConstants {

// New Track layout (always 3 effects)
// .NNN.II.111222333. = 18 chars
// NNN - note column (3 cells)
// II - instrument columns (2 columns, 2 cells)
// 111 - effect 1 columns (3 columns, 3 cells)
// 222 - effect 2 columns (3 columns, 3 cells)
// 333 - effect 3 columns (3 columns, 3 cells)

// converts a column index -> cell index
uint8_t TRACK_COLUMN_MAP[] = {
    // .NNN.II.111.222.333.
    //  0   12 345 678 9AB
    1,  // +PatternColumn::note
    5,  // +PatternColumn::instrumentHigh
    6,  // +PatternColumn::instrumentLow
    8,  // PatternCursor::ColumnEffect1Type
    9,  // PatternCursor::ColumnEffect1ArgHigh
    10, // PatternCursor::ColumnEffect1ArgLow
    12, // PatternCursor::ColumnEffect2Type
    13, // PatternCursor::ColumnEffect2ArgHigh
    14, // PatternCursor::ColumnEffect2ArgLow
    16, // PatternCursor::ColumnEffect3Type
    17, // PatternCursor::ColumnEffect3ArgHigh
    18  // PatternCursor::ColumnEffect3ArgLow

};

// converts a cell index -> column index
uint8_t TRACK_CELL_MAP[] = {
    PatternCursor::ColumnNote,    // spacing
    PatternCursor::ColumnNote,
    PatternCursor::ColumnNote,
    PatternCursor::ColumnNote,

    PatternCursor::ColumnInstrumentHigh, // spacing
    PatternCursor::ColumnInstrumentHigh,
    PatternCursor::ColumnInstrumentLow,

    PatternCursor::ColumnEffect1Type, // spacing
    PatternCursor::ColumnEffect1Type,
    PatternCursor::ColumnEffect1ArgHigh,
    PatternCursor::ColumnEffect1ArgLow,
    PatternCursor::ColumnEffect2Type, // spacing
    PatternCursor::ColumnEffect2Type,
    PatternCursor::ColumnEffect2ArgHigh,
    PatternCursor::ColumnEffect2ArgLow,
    PatternCursor::ColumnEffect3Type, // spacing
    PatternCursor::ColumnEffect3Type,
    PatternCursor::ColumnEffect3ArgHigh,
    PatternCursor::ColumnEffect3ArgLow,
    PatternCursor::ColumnEffect3ArgLow // spacing
};

uint8_t TRACK_CELL_TO_DATA_COLUMN[] = {
    0,    // spacing
    0,
    0,
    0,

    1, // spacing
    1,
    1,

    2, // spacing
    2,
    2,
    2,
    3, // spacing
    3,
    3,
    3,
    4, // spacing
    4,
    4,
    4,
    4 // spacing
};

}
