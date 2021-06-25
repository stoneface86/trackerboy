
#pragma once

#include <type_traits>

struct PatternCursorBase {

    int row;
    int column;
    int track;

    static constexpr int MAX_TRACKS = 4;

protected:
    constexpr PatternCursorBase() :
        row(0),
        column(0),
        track(0)
    {
    }

    constexpr PatternCursorBase(int row, int column, int track) :
        row(row),
        column(column),
        track(track)
    {
    }

    template <int columnMax>
    constexpr bool isValid() {
        return row >= 0 &&
               column >= 0 && column < columnMax &&
               track >= 0 && track < MAX_TRACKS;
    }

};


//
// Container class for the PatternModel's cursor
//
struct PatternCursor : public PatternCursorBase {

    constexpr PatternCursor() :
        PatternCursorBase() 
    {
    }

    constexpr PatternCursor(int row, int column, int track) :
        PatternCursorBase(row, column, track)
    {
    }

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
    

    constexpr bool isValid() {
        return PatternCursorBase::isValid<MAX_COLUMNS>();
    }


};

//
// A cursor type for PatternSelection. Same as PatternCursor, except the column
// member is a select column.
//
struct PatternAnchor : public PatternCursorBase {

    //
    // Selectable columns
    //
    enum SelectType {
        SelectNote,
        SelectInstrument,
        SelectEffect1,
        SelectEffect2,
        SelectEffect3
    };

    static constexpr SelectType selectColumn(int column) {
        switch (column) {
            case PatternCursor::ColumnNote:
                return PatternAnchor::SelectNote;

            case PatternCursor::ColumnInstrumentHigh:
            case PatternCursor::ColumnInstrumentLow:
                return PatternAnchor::SelectInstrument;

            case PatternCursor::ColumnEffect1Type:
            case PatternCursor::ColumnEffect1ArgHigh:
            case PatternCursor::ColumnEffect1ArgLow:
                return PatternAnchor::SelectEffect1;

            case PatternCursor::ColumnEffect2Type:
            case PatternCursor::ColumnEffect2ArgHigh:
            case PatternCursor::ColumnEffect2ArgLow:
                return PatternAnchor::SelectEffect2;

            default:
                return PatternAnchor::SelectEffect3;
        }
    }


    constexpr PatternAnchor() :
        PatternCursorBase() 
    {
    }

    constexpr PatternAnchor(int row, int column, int track) :
        PatternCursorBase(row, column, track)
    {
    }

    // this constructor is implicit, so that you can pass a PatternCursor to
    // PatternSelection methods and it will be automatically converted
    constexpr PatternAnchor(PatternCursor pos) :
        PatternCursorBase(
            pos.row,
            (int)selectColumn(pos.column),
            pos.track
        )
    {
    }

    static constexpr int MAX_SELECTS = SelectEffect3 + 1;

    constexpr bool isValid() {
        return PatternCursorBase::isValid<MAX_SELECTS>();
    }


};

// these overloads can only be used with PatternAnchor or PatternCursor types
// (or PatternCursorBase, but it's not possible to construct one of those)
// both the lhs and rhs must be the same type

template <
    class T,
    typename std::enable_if<std::is_base_of<PatternCursorBase, T>::value>::type* = nullptr
>
inline constexpr bool operator==(T const& lhs, T const& rhs) {
    return lhs.row == rhs.row &&
           lhs.column == rhs.column &&
           lhs.track == rhs.track;
}

template <
    class T,
    typename std::enable_if<std::is_base_of<PatternCursorBase, T>::value>::type* = nullptr
>
inline constexpr bool operator!=(T const& lhs, T const& rhs) {
    return lhs.row != rhs.row ||
           lhs.column != rhs.column ||
           lhs.track != rhs.track;
}
