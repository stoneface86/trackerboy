
#pragma once

#include "core/PatternCursor.hpp"

#include <QtDebug>

/*
Developer notes for iterating selections:

When modifying a pattern, you will typically apply an operation to a
selection. The Iterator class is a utility for doing so.

To iterate a selection, first get an iterator from the selection

auto iter = selection.iterator();

then you can iterate over the selected tracks via trackStart() and trackEnd()
and/or over the selected rows via rowStart() and rowEnd()

For checking the selected columns, you can get a TrackMeta object for the
given track, and use hasColumn()

example code:

auto iter = selection.iterator();
for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
    auto tmeta = iter.getTrackMeta(track);

    for (auto row = iter.rowStart(); row <= iter.rowEnd(); ++row) {
        if (tmeta.hasColumn<PatternSelection::SelectNote>()) {
            // do something for the note column
        }
        if (tmeta.hasColumn<PatternSelection::SelectInstrument>()) {
            // do something for the instrument column
        }

        // effects can also be iterated via for loop
        for (int effectNo = tmeta.effectStart(); effectNo < tmeta.effectEnd(); ++effectNo) {
            // do something for each effect
        }

        // or via hasColumn
        if (tmeta.hasColumn<PatternSelection::SelectEffect1>()) {
            // do something for effect 1
        }
    }
}

Note that the end indices for all coordinates are inclusive, so use <= in your
loop condition. (Except for effect columns, TrackMeta::effectEnd() is exclusive)

*/


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
        int mEffectStart;
        int mEffectEnd;

        constexpr TrackMeta(int start, int end) :
            mStart(start),
            mEnd(end),
            mEffectStart(std::max(0, start - SelectEffect1)),
            mEffectEnd(std::max(0, end - SelectEffect1 + 1))
        {
        }

    public:

        constexpr int columnStart() const {
            return mStart;
        }

        constexpr int columnEnd() const {
            return mEnd;
        }

        // starting effect column that is selected
        constexpr int effectStart() const {
            return mEffectStart;
        }

        // ending effect column + 1 that is selected
        constexpr int effectEnd() const {
            return mEffectEnd;
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

        constexpr PatternCursor start() const {
            return mStart;
        }

        constexpr PatternCursor end() const {
            return mEnd;
        }

        TrackMeta getTrackMeta(int track) const;

    };

    PatternSelection();
    PatternSelection(PatternCursor pos);
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

    //
    // Adjusts the selection such that both cursors are valid positions in the
    // pattern.
    //
    void clamp(int rowMax);

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
