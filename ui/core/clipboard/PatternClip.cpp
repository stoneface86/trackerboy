
#include "core/clipboard/PatternClip.hpp"

#include <QtGlobal>

#include <type_traits>

PatternClip::PatternClip() :
    mData(),
    mLocation()
{
}

bool PatternClip::hasData() const {
    return bool(mData);
}

PatternSelection const& PatternClip::selection() {
    return mLocation;
}

void PatternClip::restore(trackerboy::Pattern &dest) {
    if (mData) {
        move<false>(dest);
    }
}

void PatternClip::save(trackerboy::Pattern const& src, PatternSelection region) {
    mLocation = region;
    move<true>(const_cast<trackerboy::Pattern&>(src));
}



// might want to make this a table instead
size_t columnToOffset(int column) {
    switch (column) {
        case PatternSelection::SelectNote:
            return offsetof(trackerboy::TrackRow, note);
        case PatternSelection::SelectInstrument:
            return offsetof(trackerboy::TrackRow, instrumentId);
        case PatternSelection::SelectEffect1:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 0;
        case PatternSelection::SelectEffect2:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 1;
        default:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 2;
    }
}

size_t columnToLength(int column) {
    if (++column == PatternSelection::MAX_SELECTS) {
        return sizeof(trackerboy::TrackRow);
    } else {
        return columnToOffset(column);
    }
}

size_t getRowLength(PatternSelection::Iterator const& iter) {

    auto trackStart = iter.trackStart();
    auto trackEnd = iter.trackEnd();
    auto tracks = trackEnd - trackStart;
    
    size_t length = sizeof(trackerboy::TrackRow) * (trackEnd - trackStart);
    
    length += columnToLength(iter.columnEnd());

    return length - columnToOffset(iter.columnStart());
}


template <bool tSaving>
void PatternClip::move(trackerboy::Pattern &pattern) {

    using TrackRowRef = std::conditional_t<tSaving, trackerboy::TrackRow const&, trackerboy::TrackRow&>;
    using patternptr_t = std::conditional_t<tSaving, const char*, char*>;
    using clipptr_t = std::conditional_t<tSaving, char*, const char*>;

    auto iter = mLocation.iterator();

    // determine the row length
    size_t rowLength = getRowLength(iter);
    
    if constexpr (tSaving) {
        mData.reset(new char[rowLength * iter.rows()]);
    }

    clipptr_t buf = mData.get();
    for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
        auto tmeta = iter.getTrackMeta(track);
        auto offset = columnToOffset(tmeta.columnStart());
        auto length = columnToLength(tmeta.columnEnd()) - offset;
        // with this assertion passing, we will never read or write past
        // the bounds of a TrackRow
        Q_ASSERT(offset + length <= sizeof(trackerboy::TrackRow));

        auto chunkptr = buf;
        for (int row = iter.rowStart(); row <= iter.rowEnd(); ++row) {
            
            TrackRowRef rowdata = pattern.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
            auto patternptr = reinterpret_cast<patternptr_t>(&rowdata) + offset;
            
            if constexpr (tSaving) {
                std::copy_n(patternptr, length, chunkptr);
            } else {
                std::copy_n(chunkptr, length, patternptr);
            }
            
            chunkptr += rowLength;
        }

        // advance to next track
        buf += length;
    }

}
