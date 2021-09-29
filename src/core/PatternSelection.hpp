
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

auto const iter = selection.iterator();
for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
    auto const tmeta = iter.getTrackMeta(track);

    for (auto row = iter.rowStart(); row <= iter.rowEnd(); ++row) {
        if (tmeta.hasColumn<PatternAnchor::SelectNote>()) {
            // do something for the note column
        }
        if (tmeta.hasColumn<PatternAnchor::SelectInstrument>()) {
            // do something for the instrument column
        }

        // effects can also be iterated via for loop
        for (int effectNo = tmeta.effectStart(); effectNo < tmeta.effectEnd(); ++effectNo) {
            // do something for each effect
        }

        // or via hasColumn
        if (tmeta.hasColumn<PatternAnchor::SelectEffect1>()) {
            // do something for effect 1
        }
    }
}

Note that the end indices for all coordinates are inclusive, so use <= in your
loop condition. (Except for effect columns, TrackMeta::effectEnd() is exclusive)

Also note that a PatternSelection uses PatternAnchors and not PatternCursors.
You can still use cursors, however, they will be implicitly converted to an
anchor. Iterators also use anchors, so the columns you are iterating are select
columns and not cursor columns (PatternAnchor::SelectType instead of
PatternCursor::ColumnType).
*/


//
// Class representing selected data for a pattern. Contains two PatternAnchors
// start and end.
//
class PatternSelection {

public:

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
            mEffectStart(std::max(0, start - PatternAnchor::SelectEffect1)),
            mEffectEnd(std::max(0, end - PatternAnchor::SelectEffect1 + 1))
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
        template <PatternAnchor::SelectType tColumn>
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
        PatternAnchor mStart;
        PatternAnchor mEnd;

        constexpr Iterator(PatternAnchor start, PatternAnchor end) :
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

        constexpr PatternAnchor start() const {
            return mStart;
        }

        constexpr PatternAnchor end() const {
            return mEnd;
        }

        TrackMeta getTrackMeta(int track) const;

    };

    PatternSelection();

    PatternSelection(PatternAnchor anchor);
    PatternSelection(PatternAnchor start, PatternAnchor end);
    

    PatternAnchor start() const;

    PatternAnchor end() const;

    void setStart(PatternAnchor start);

    void setEnd(PatternAnchor end);

    //
    // Gets an interator for this selection
    //
    Iterator iterator() const;

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
    void moveTo(PatternAnchor pos);

    //
    // Determine if the given cursor is within this selection
    //
    bool contains(PatternAnchor cursor);

private:

    PatternAnchor mStart;
    PatternAnchor mEnd;

#ifndef QT_NO_DEBUG
friend QDebug operator<<(QDebug debug, const PatternSelection &selection);
#endif


};
