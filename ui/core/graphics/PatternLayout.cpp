
#include "core/graphics/PatternLayout.hpp"

#include <algorithm>

#define TU PatternLayoutTU
namespace TU {

struct ColumnLocator {
    int flatOffset;
    int cells;

    constexpr ColumnLocator(int spacings, int cells) :
        flatOffset(spacings * PatternLayout::SPACING),
        cells(cells)
    {
    }

    constexpr int locate(int cellWidth) const noexcept {
        return flatOffset + (cellWidth * cells);
    }

};

static ColumnLocator const COLUMN_LAYOUT[] = {
    ColumnLocator(1, 0), // ColumnNote
    ColumnLocator(2, 3), // ColumnInstrumentHigh
    ColumnLocator(2, 4), // ColumnInstrumentLow
    ColumnLocator(3, 5), // ColumnEffect1Type
    ColumnLocator(3, 6), // ColumnEffect1ArgHigh
    ColumnLocator(3, 7), // ColumnEffect1ArgLow
    ColumnLocator(4, 8), // ColumnEffect1Type
    ColumnLocator(4, 9), // ColumnEffect1ArgHigh
    ColumnLocator(4, 10), // ColumnEffect1ArgLow
    ColumnLocator(5, 11), // ColumnEffect1Type
    ColumnLocator(5, 12), // ColumnEffect1ArgHigh
    ColumnLocator(5, 13), // ColumnEffect1ArgLow

};

static int const SELECT_TO_COLUMN[] = {
    PatternCursor::ColumnNote,
    PatternCursor::ColumnInstrumentHigh,
    PatternCursor::ColumnEffect1Type,
    PatternCursor::ColumnEffect2Type,
    PatternCursor::ColumnEffect3Type
};

static int const SELECT_WIDTHS[] = {
    3,
    2,
    3,
    3,
    3
};

int locateSelect(int selectColumn, int cellWidth) {
    return COLUMN_LAYOUT[SELECT_TO_COLUMN[selectColumn]].locate(cellWidth);
}

} // namespace TU


PatternLayout::PatternLayout() :
    mCellWidth(0),
    mCellHeight(0),
    mPatternStart(0),
    mTrackWidth(0),
    mPatternWidth(0)
{
}

void PatternLayout::setCellSize(int width, int height) {
    if (width != mCellWidth) {
        mCellWidth = width;

        // the row number column
        // _XX_|
        // _ : spacing
        // X : hex cell
        // | : line

        mPatternStart = (SPACING * 2) + (width * 2) + LINE_WIDTH;

        // track width calculation
        // _NNN_II_EXX_EXX_EXX_
        // 6 spacings
        // 14 cells
        mTrackWidth = (SPACING * 6) + (width * 14);
        mPatternWidth = mTrackWidth * 4 + (LINE_WIDTH * 3);

        constexpr auto HALF_SPACING = SPACING / 2;
        #define calcDiv(column, spacing) \
            mColumnDivs[column] = TU::COLUMN_LAYOUT[column + 1].locate(width) - spacing
        
        calcDiv(PatternCursor::ColumnNote, HALF_SPACING);
        calcDiv(PatternCursor::ColumnInstrumentHigh, 0);
        calcDiv(PatternCursor::ColumnInstrumentLow, HALF_SPACING);
        calcDiv(PatternCursor::ColumnEffect1Type, 0);
        calcDiv(PatternCursor::ColumnEffect1ArgHigh, 0);
        calcDiv(PatternCursor::ColumnEffect1ArgLow, HALF_SPACING);
        calcDiv(PatternCursor::ColumnEffect2Type, 0);
        calcDiv(PatternCursor::ColumnEffect2ArgHigh, 0);
        calcDiv(PatternCursor::ColumnEffect2ArgLow, HALF_SPACING);
        calcDiv(PatternCursor::ColumnEffect3Type, 0);
        calcDiv(PatternCursor::ColumnEffect3ArgHigh, 0);
        
        #undef calcDiv
    }

    mCellHeight = height;
}

int PatternLayout::patternStart() const {
    return mPatternStart;
}

int PatternLayout::trackWidth(int track) const {
    (void)track;
    // currently all tracks have the same width, this will change once
    // dynamic effect column counts are implemented
    return mTrackWidth;
}

int PatternLayout::rowWidth() const {
    return mPatternWidth;
}

int PatternLayout::trackToX(int track) const {
    Q_ASSERT(track >= 0 && track < PatternCursor::MAX_TRACKS);
    return mPatternStart + ((mTrackWidth + LINE_WIDTH) * track);
}

int PatternLayout::columnToX(int column) const {
    Q_ASSERT(column >= 0 && column < PatternCursor::MAX_COLUMNS);
    return TU::COLUMN_LAYOUT[column].locate(mCellWidth);
}

PatternCursor PatternLayout::mouseToCursor(QPoint point) const {

    auto x = point.x() - mPatternStart;

    // since all tracks are the same width, we can just divide to find
    // the track, this won't work for dynamic effect column counts.
    int track = x / mTrackWidth;
    int xInTrack = x % mTrackWidth;
    int column = 0;

    // linear search from start of array
    // would be faster (on average) to start in the middle but whatever
    for (auto const div : mColumnDivs) {
        if (xInTrack < div) {
            break;
        }
        ++column;
    }
    return {
        0,
        column,
        track
    };
    


}

QRect PatternLayout::selectionRectangle(PatternSelection const& selection) const {

    auto iter = selection.iterator();

    int x1 = TU::locateSelect(iter.columnStart(), mCellWidth) - SPACING;
    int x2 = TU::locateSelect(iter.columnEnd(), mCellWidth) + SPACING;
    x2 += mCellWidth * TU::SELECT_WIDTHS[iter.columnEnd()];

    x1 += mPatternStart + (iter.trackStart() * (mTrackWidth + LINE_WIDTH));
    x2 += mPatternStart + (iter.trackEnd() * (mTrackWidth + LINE_WIDTH));

    return {
        x1,
        iter.rowStart() * mCellHeight,
        x2 - x1,
        iter.rows() * mCellHeight
    };
}

#undef TU
