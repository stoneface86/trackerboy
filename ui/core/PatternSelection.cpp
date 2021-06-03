
#include "core/PatternSelection.hpp"

#include <algorithm>

PatternSelection::SelectType PatternSelection::selectColumn(int column) {
    switch (column) {
        case PatternCursor::ColumnNote:
            return PatternSelection::SelectNote;

        case PatternCursor::ColumnInstrumentHigh:
        case PatternCursor::ColumnInstrumentLow:
            return PatternSelection::SelectInstrument;

        case PatternCursor::ColumnEffect1Type:
        case PatternCursor::ColumnEffect1ArgHigh:
        case PatternCursor::ColumnEffect1ArgLow:
            return PatternSelection::SelectEffect1;

        case PatternCursor::ColumnEffect2Type:
        case PatternCursor::ColumnEffect2ArgHigh:
        case PatternCursor::ColumnEffect2ArgLow:
            return PatternSelection::SelectEffect2;

        default:
            return PatternSelection::SelectEffect3;
    }
}

PatternSelection::PatternSelection() :
    mStart(),
    mEnd()
{
}

PatternSelection::PatternSelection(PatternCursor start, PatternCursor end) :
    mStart(start),
    mEnd(end)
{
}

PatternCursor PatternSelection::start() const {
    return mStart;
}

PatternCursor PatternSelection::end() const {
    return mEnd;
}

void PatternSelection::setStart(PatternCursor start) {
    mStart = start;
}

void PatternSelection::setEnd(PatternCursor end) {
    mEnd = end;
}

PatternSelection::Iterator PatternSelection::iterator() const {
    PatternCursor start = mStart;
    PatternCursor end = mEnd;

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
        end = MAX_SELECTS - 1;
    }

    return { start, end };
}

void PatternSelection::translate(int rows) {
    mStart.row += rows;
    mEnd.row += rows;
}

#ifndef QT_NO_DEBUG

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
