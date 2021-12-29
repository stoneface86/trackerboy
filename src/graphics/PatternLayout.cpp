
#include "graphics/PatternLayout.hpp"

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
    mTrackWidths(),
    mTrackStarts(),
    mPatternWidth(0),
    mRownoCells(2),
    mEffectCounts(trackerboy::DEFAULT_EFFECT_COUNTS),
    mColumnDivs()
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

        // track width calculation
        // _NNN_II_EXX_EXX_EXX_
        // 6 spacings
        // 14 cells
        auto const minWidth = (SPACING * 3) + (5 * width);
        auto const effectWidth = SPACING + (3 * width);
        mPatternWidth = (LINE_WIDTH * 3);
        for (size_t i = 0; i < mTrackWidths.size(); ++i) {
            int trackWidth = minWidth + (effectWidth * mEffectCounts[i]);
            mPatternWidth += trackWidth;
            mTrackWidths[i] = trackWidth;
        }

        {
            int x = (SPACING * 2) + (width * mRownoCells) + LINE_WIDTH;
            mTrackStarts[0] = x;
            for (size_t i = 1; i < mTrackStarts.size(); ++i) {
                x += mTrackWidths[i - 1] + LINE_WIDTH;
                mTrackStarts[i] = x;
                
            }
        }

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

bool PatternLayout::rownoHex() const {
    return mRownoCells == 2;
}

void PatternLayout::setRownoHex(bool hex) {
    setRownoCells(hex ? 2 : 3);

}

int PatternLayout::effectsVisible(int track) const {
    return mEffectCounts[track];
}

void PatternLayout::setEffectsVisible(int track, int count) {
    Q_ASSERT(track >= 0 && track < PatternCursor::MAX_TRACKS);
    Q_ASSERT(count >= 1 && count <= 3);

    int countDiff = count - mEffectCounts[track];
    if (countDiff) {
        mEffectCounts[track] = (char)count;

        // calculate width difference
        // reminder: an effect column is 3 cells and a spacing
        int diff = (mCellWidth * 3 + SPACING) * countDiff;
        // update width for the track
        mTrackWidths[track] += diff;

        // recalculate starting positions, add the difference to all tracks
        // ahead of this one
        for (auto iter = mTrackStarts.begin() + track + 1; iter != mTrackStarts.end(); ++iter) {
            *iter += diff;
        }
        mPatternWidth += diff;
    }
}

void PatternLayout::setRownoCells(int cells) {
    auto diff = (cells - mRownoCells) * mCellWidth;
    if (diff) {
        mRownoCells = cells;
        for (auto &start : mTrackStarts) {
            start += diff;
        }
    }
}

int PatternLayout::patternStart() const {
    return mTrackStarts[0];
}

int PatternLayout::trackWidth(int track) const {
    return mTrackWidths[track];
}

int PatternLayout::rowWidth() const {
    return mPatternWidth;
}

int PatternLayout::trackToX(int track) const {
    Q_ASSERT(track >= 0 && track < PatternCursor::MAX_TRACKS);
    return mTrackStarts[track];
}

int PatternLayout::columnToX(int column) const {
    Q_ASSERT(column >= 0 && column < PatternCursor::MAX_COLUMNS);
    return TU::COLUMN_LAYOUT[column].locate(mCellWidth);
}

PatternCursor PatternLayout::mouseToCursor(QPoint point) const {

    auto x = point.x();

    int track = mouseToTrack(x);
    int column = -1;

    if (track >= 0 && track < PatternCursor::MAX_TRACKS) {
        column = 0;
        int xInTrack = x - mTrackStarts[track];
        // linear search from start of array
        // would be faster (on average) to start in the middle but whatever (see mouseToTrack implementation)
        // Optimize if this is a hot path
        for (auto const div : mColumnDivs) {
            if (xInTrack < div) {
                break;
            }
            ++column;
        }
        column = std::min(maxColumnInTrack(track), column);
        
    }

    return {
        0,
        column,
        track
    };
    


}

int PatternLayout::mouseToTrack(int x) const {
    // starting in the middle results in 2-3 branches
    if (x >= mTrackStarts[2]) {
        if (x < mTrackStarts[3]) {
            return 2;
        } else if (x < mTrackStarts[4]) {
            return 3;
        }

        return 4;
    } else {
        if (x >= mTrackStarts[1]) {
            return 1;
        } else if (x >= mTrackStarts[0]) {
            return 0;
        }
        return -1;
    }
    
}

QRect PatternLayout::selectionRectangle(PatternSelection const& selection) const {

    auto iter = selection.iterator();

    int x1 = TU::locateSelect(iter.columnStart(), mCellWidth) - SPACING;
    auto const maxSelect = mEffectCounts[iter.trackEnd()] + PatternAnchor::SelectInstrument;
    auto const endSelect = std::min(maxSelect, iter.columnEnd());
    int x2 = TU::locateSelect(endSelect, mCellWidth) + SPACING;
    x2 += mCellWidth * TU::SELECT_WIDTHS[iter.columnEnd()];

    x1 += mTrackStarts[iter.trackStart()];
    x2 += mTrackStarts[iter.trackEnd()];

    return {
        x1,
        iter.rowStart() * mCellHeight,
        x2 - x1,
        iter.rows() * mCellHeight
    };
}

int PatternLayout::maxColumnInTrack(int track) const {
    // columns in a track:
    // 1 - note
    // 2 - instrument hi/low
    // 3 - effect 1
    // 3 - effect 2
    // 3 - effect 3

    // then the maximum column in a track is given by
    // note + instrument + (n * effect) - 1
    // ==> 1 + 2 + 3n - 1
    // ==> 3n + 2
    // where n is the number of effects visible
    return mEffectCounts[track] * 3 + 2;
}

#undef TU
