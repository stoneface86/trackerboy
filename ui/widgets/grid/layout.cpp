
#include "widgets/grid/layout.hpp"

#include "core/model/PatternModel.hpp"

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
    8,  // PatternModel::ColumnEffect1Type
    9,  // PatternModel::ColumnEffect1ArgHigh
    10, // PatternModel::ColumnEffect1ArgLow
    12, // PatternModel::ColumnEffect2Type
    13, // PatternModel::ColumnEffect2ArgHigh
    14, // PatternModel::ColumnEffect2ArgLow
    16, // PatternModel::ColumnEffect3Type
    17, // PatternModel::ColumnEffect3ArgHigh
    18  // PatternModel::ColumnEffect3ArgLow

};

// converts a cell index -> column index
uint8_t TRACK_CELL_MAP[] = {
    PatternModel::ColumnNote,    // spacing
    PatternModel::ColumnNote,
    PatternModel::ColumnNote,
    PatternModel::ColumnNote,

    PatternModel::ColumnInstrumentHigh, // spacing
    PatternModel::ColumnInstrumentHigh,
    PatternModel::ColumnInstrumentLow,

    PatternModel::ColumnEffect1Type, // spacing
    PatternModel::ColumnEffect1Type,
    PatternModel::ColumnEffect1ArgHigh,
    PatternModel::ColumnEffect1ArgLow,
    PatternModel::ColumnEffect2Type, // spacing
    PatternModel::ColumnEffect2Type,
    PatternModel::ColumnEffect2ArgHigh,
    PatternModel::ColumnEffect2ArgLow,
    PatternModel::ColumnEffect3Type, // spacing
    PatternModel::ColumnEffect3Type,
    PatternModel::ColumnEffect3ArgHigh,
    PatternModel::ColumnEffect3ArgLow,
    PatternModel::ColumnEffect3ArgLow // spacing
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
