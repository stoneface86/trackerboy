
#include "widgets/grid/layout.hpp"

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
    8,  // COLUMN_EFFECT1_TYPE
    9,  // COLUMN_EFFECT1_ARG_HIGH
    10, // COLUMN_EFFECT1_ARG_LOW
    12, // COLUMN_EFFECT2_TYPE
    13, // COLUMN_EFFECT2_ARG_HIGH
    14, // COLUMN_EFFECT2_ARG_LOW
    16, // COLUMN_EFFECT3_TYPE
    17, // COLUMN_EFFECT3_ARG_HIGH
    18  // COLUMN_EFFECT3_ARG_LOW

};

// converts a cell index -> column index
uint8_t TRACK_CELL_MAP[] = {
    COLUMN_NOTE,    // spacing
    COLUMN_NOTE,
    COLUMN_NOTE,
    COLUMN_NOTE,

    COLUMN_INSTRUMENT_HIGH, // spacing
    COLUMN_INSTRUMENT_HIGH,
    COLUMN_INSTRUMENT_LOW,

    COLUMN_EFFECT1_TYPE, // spacing
    COLUMN_EFFECT1_TYPE,
    COLUMN_EFFECT1_ARG_HIGH,
    COLUMN_EFFECT1_ARG_LOW,
    COLUMN_EFFECT2_TYPE, // spacing
    COLUMN_EFFECT2_TYPE,
    COLUMN_EFFECT2_ARG_HIGH,
    COLUMN_EFFECT2_ARG_LOW,
    COLUMN_EFFECT3_TYPE, // spacing
    COLUMN_EFFECT3_TYPE,
    COLUMN_EFFECT3_ARG_HIGH,
    COLUMN_EFFECT3_ARG_LOW,
    COLUMN_EFFECT3_ARG_LOW // spacing
};

}
