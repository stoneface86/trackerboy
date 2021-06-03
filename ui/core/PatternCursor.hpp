
#pragma once


//
// Container class for the PatternModel's cursor
//
struct PatternCursor {

    //
    // enum for editable columns
    //
    enum ColumnType {
        ColumnNote,

        // high is the upper nibble (bits 4-7)
        // low is the lower nibble (bits 0-3)

        ColumnInstrumentHigh,
        ColumnInstrumentLow,

        ColumnEffect1Type,
        ColumnEffect1ArgHigh,
        ColumnEffect1ArgLow,

        ColumnEffect2Type,
        ColumnEffect2ArgHigh,
        ColumnEffect2ArgLow,

        ColumnEffect3Type,
        ColumnEffect3ArgHigh,
        ColumnEffect3ArgLow,

    };

    static constexpr int MAX_COLUMNS = ColumnEffect3ArgLow + 1;
    static constexpr int MAX_TRACKS = 4;

    int row;
    int column;
    int track;

    constexpr PatternCursor() :
        row(0),
        column(0),
        track(0)
    {
    }

    constexpr PatternCursor(int row, int column, int track) :
        row(row),
        column(column),
        track(track)
    {
    }

    constexpr bool isValid() {
        return row >= 0 &&
               column >= 0 && column < MAX_COLUMNS &&
               track >= 0 && track < MAX_TRACKS;
    }


};

inline constexpr bool operator==(PatternCursor const& lhs, PatternCursor const& rhs) {
    return lhs.row == rhs.row &&
            lhs.column == rhs.column &&
            lhs.track == rhs.track;
}

inline constexpr bool operator!=(PatternCursor const& lhs, PatternCursor const& rhs) {
    return lhs.row != rhs.row ||
            lhs.column != rhs.column ||
            lhs.track != rhs.track;
}
