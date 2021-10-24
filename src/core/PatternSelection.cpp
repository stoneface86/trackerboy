
#include "core/PatternSelection.hpp"

#include <algorithm>


PatternSelection::PatternSelection() :
    mStart(),
    mEnd()
{
}

PatternSelection::PatternSelection(PatternAnchor anchor) :
    mStart(anchor),
    mEnd(anchor)
{
}

PatternSelection::PatternSelection(PatternAnchor start, PatternAnchor end) :
    mStart(start),
    mEnd(end)
{
}

PatternAnchor PatternSelection::start() const {
    return mStart;
}

PatternAnchor PatternSelection::end() const {
    return mEnd;
}

void PatternSelection::setStart(PatternAnchor start) {
    mStart = start;
}

void PatternSelection::setEnd(PatternAnchor end) {
    mEnd = end;
}

PatternSelection::Iterator PatternSelection::iterator() const {
    auto start = mStart;
    auto end = mEnd;

    // normalize coordinates

    if (start.row > end.row) {
        std::swap(start.row, end.row);
    }


    if (start.track > end.track) {
        std::swap(start.track, end.track);
        std::swap(start.column, end.column);
    } else if (start.track == end.track && start.column > end.column) {
        std::swap(start.column, end.column);
    }
    

    return {start, end};
}

PatternSelection::TrackMeta PatternSelection::Iterator::getTrackMeta(int track) const {
    int start;
    int end;

    if (track == mStart.track) {
        start = mStart.column;
    } else {
        start = 0;
    }

    if (track == mEnd.track) {
        end = mEnd.column;
    } else {
        end = PatternAnchor::MAX_SELECTS - 1;
    }

    return { start, end };
}

void PatternSelection::translate(int rows) {
    mStart.row += rows;
    mEnd.row += rows;
}

void PatternSelection::clamp(int rowMax) {
    for (auto cursor : {&mStart, &mEnd}) {
        cursor->row = std::clamp(cursor->row, 0, rowMax);
        if (cursor->track < 0) {
            cursor->track = 0;
            cursor->column = 0;
        } else if (cursor->track >= PatternAnchor::MAX_TRACKS) {
            cursor->track = PatternAnchor::MAX_TRACKS - 1;
            cursor->column = PatternAnchor::MAX_SELECTS - 1;
        } else {
            cursor->column = std::clamp(cursor->column, 0, PatternAnchor::MAX_SELECTS - 1);
        }
        
    }
}

static bool isEffect(int select) {
    return select >= PatternAnchor::SelectEffect1;
}


void PatternSelection::moveTo(PatternAnchor cursor) {
    auto iter = iterator();
    // normalize the selection coordinates
    mStart = iter.mStart;
    mEnd = iter.mEnd;

    // move to row
    mStart.row = cursor.row;
    mEnd.row = cursor.row + iter.rows() - 1;

    // if the selection is just effects, we can move it to another effect column
    // otherwise we can only move by tracks
    if (iter.trackStart() == iter.trackEnd()) {
        // within the same track
        if (isEffect(iter.columnStart())) {
            // only effects are selected, move by column
            auto start = std::max((int)PatternAnchor::SelectEffect1, cursor.column);
            int end = start + iter.columnEnd() - iter.columnStart();
            mStart.column = start;
            mEnd.column = end;
        }
    }

    // move to track
    mStart.track = cursor.track;
    mEnd.track = cursor.track + iter.trackEnd() - iter.trackStart();
}

bool PatternSelection::contains(PatternAnchor cursor) {
    auto iter = iterator();
    auto start = iter.trackStart() * PatternAnchor::MAX_SELECTS + iter.columnStart();
    auto end = iter.trackEnd() * PatternAnchor::MAX_SELECTS + iter.columnEnd();
    auto cursorColumn = cursor.track * PatternAnchor::MAX_SELECTS + cursor.column;
    return cursor.row >= iter.rowStart() && cursor.row <= iter.rowEnd() &&
           cursorColumn >= start && cursorColumn <= end;
}

#ifdef QT_DEBUG

QDebug operator<<(QDebug debug, PatternSelection const& selection) {
    QDebugStateSaver saver(debug);
    debug.nospace() << QStringLiteral("PatternSelection({%1, %2, %3}, {%4, %5, %6})")
        .arg(selection.mStart.row)
        .arg(selection.mStart.column)
        .arg(selection.mStart.track)
        .arg(selection.mEnd.row)
        .arg(selection.mEnd.column)
        .arg(selection.mEnd.track);
    return debug;
}

#endif
