
#pragma once

#include "core/PatternCursor.hpp"

#include <QtDebug>

//
// Class representing selected data for a pattern. Contains two cursor positions
// start and end.
//
class PatternSelection {

public:

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

    static constexpr int MAX_SELECTS = SelectEffect3 + 1;

    class Iterator;

    // utility for iterating a track
    class TrackMeta {
        friend class Iterator;

        int mStart;
        int mEnd;

        constexpr TrackMeta(int start, int end) :
            mStart(start),
            mEnd(end)
        {
        }

    public:

        constexpr int columnStart() const {
            return mStart;
        }

        constexpr int columnEnd() const {
            return mEnd;
        }

        //
        // Returns true if the given column is selected
        //
        template <SelectType tColumn>
        constexpr bool hasColumn() const {
            return tColumn >= mStart && tColumn <= mEnd;
        }

    };

    //
    // Nothing like a std c++ iterator, just a utility for iterating the selection
    //
    class Iterator {

        friend class PatternSelection;

        // same as Selection, but mStart is <= mEnd
        PatternCursor mStart;
        PatternCursor mEnd;

        constexpr Iterator(PatternCursor start, PatternCursor end) :
            mStart(start),
            mEnd(end)
        {
        }

    public:

        constexpr int columnStart() const {
            return mStart.column;
        }

        constexpr int columnEnd() const {
            return mEnd.column;
        }

        constexpr int rowStart() const {
            return mStart.row;
        }

        constexpr int rowEnd() const {
            return mEnd.row;
        }

        constexpr int rows() const {
            return mEnd.row - mStart.row + 1;
        }

        constexpr int trackStart() const {
            return mStart.track;
        }

        constexpr int trackEnd() const {
            return mEnd.track;
        }

        TrackMeta getTrackMeta(int track) const;

    };

    PatternSelection();
    PatternSelection(PatternCursor start, PatternCursor end);

    PatternCursor start() const;

    PatternCursor end() const;

    void setStart(PatternCursor start);

    void setEnd(PatternCursor end);

    //
    // Gets an interator for this selection
    //
    Iterator iterator() const;

    //
    // Converts a cursor column (ColumnType) to a select column (SelectType)
    //
    static SelectType selectColumn(int column);

    //
    // Translates the selection by the given number of rows. Each coordinate's
    // row is added by the given rows parameter
    //
    void translate(int rows);

    void clampRows(int min, int max);

    //
    // Moves the selection to start at the given cursor position
    //
    void moveTo(PatternCursor cursor);

    //
    // Determine if the given cursor is within this selection
    //
    bool contains(PatternCursor cursor);

private:

    // the columns in these cursors are select columns (SelectType)
    PatternCursor mStart;
    PatternCursor mEnd;

#ifndef QT_NO_DEBUG
friend QDebug operator<<(QDebug debug, const PatternSelection &selection);
#endif


};
