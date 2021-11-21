
#include "clipboard/PatternClip.hpp"

#include <QtGlobal>

#include <algorithm>
#include <cstddef>

//
// Implementation details
//
// Clipped pattern data contains two parts, a PatternSelection for the location
// of the clipped data and the clip data buffer.
//
// The data buffer is just a fixed array of partial rows. The size of a
// row depends on how many columns are selected. Note and instrument columns
// are 1 byte and effect columns are 2 bytes. If I have a selection in just
// track 1 with note, instrument and effect 1 then the size of the row for this
// clip is 4 bytes. If I have 5 rows selected then this clips data buffer is
// 20 bytes long.
//
// for example, if I select this data from (0, 0, 0) to (4, 2, 0)
//      +----------------+
// 00 | | C-3   00   EF0 | ...   ...   |   ...
// 01 | | ...   ..   ... | ...   ...   |   ...
// 02 | | ==    ..   ... | ...   ...   |   ...
// 03 | | ...   ..   ... | ...   ...   |   ...
// 04 | | ...   ..   ... | ...   ...   |   ...
//    | +----------------+
// 05 |   ...   ..   ...   ...   ...   |   ...
//
// then the clip's data buffer is
// (note C-3 is 12 + 1 = 13, effect set envelope is 6)
// {
//        13, 1, 6, 0,      // row 0
//        0, 0, 0, 0        // row 1
//        85, 0, 0, 0       // row 2
//        0, 0, 0, 0        // row 3
//        0, 0, 0, 0        // row 4
// }
//
// The clip can be restored to its original location, or moved (pasted) to a
// new location. Pasting to a new location may result in a partial copy if the
// clip goes out of bounds of the destination pattern.
// 
//
//


#define TU PatternClipTU
namespace TU {

// might want to make this a table instead
// this function converts a select column to an offset in the trackerboy::TrackRow structure
size_t columnToOffset(int column) {
    switch (column) {
        case PatternAnchor::SelectNote:
            return offsetof(trackerboy::TrackRow, note);
        case PatternAnchor::SelectInstrument:
            return offsetof(trackerboy::TrackRow, instrumentId);
        case PatternAnchor::SelectEffect1:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 0;
        case PatternAnchor::SelectEffect2:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 1;
        default:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 2;
    }
}

//
// Gets the size of a column and the size of everything to the left of it
//
size_t columnToLength(int column) {
    if (++column >= PatternAnchor::MAX_SELECTS) {
        return sizeof(trackerboy::TrackRow);
    } else {
        return columnToOffset(column);
    }
}

//
// Determines the length of a row in the clip's data buffer using the given
// iterator
//
size_t getRowLength(PatternSelection::Iterator const& iter) {

    auto trackStart = iter.trackStart();
    auto trackEnd = iter.trackEnd();
    
    size_t length = sizeof(trackerboy::TrackRow) * (trackEnd - trackStart);
    
    length += columnToLength(iter.columnEnd());

    return length - columnToOffset(iter.columnStart());
}

}

PatternClip::PatternClip() :
    mData(),
    mLocation()
{
}

PatternClip::PatternClip(PatternClip const& clip) :
    PatternClip()
{
    operator=(clip);
}

PatternClip::PatternClip(PatternClip &&clip) noexcept :
    PatternClip()
{
    operator=(std::move(clip));
}

PatternClip& PatternClip::operator=(PatternClip const& clip) {
    if (clip.mData) {
        auto iter = clip.mLocation.iterator();
        auto size = TU::getRowLength(iter) * iter.rows();
        mData = std::make_unique<char[]>(size);
        std::copy_n(clip.mData.get(), size, mData.get());
        mLocation = clip.mLocation;
    } else {
        mData.reset();
        mLocation = PatternSelection();
    }

    return *this;
}

PatternClip& PatternClip::operator=(PatternClip &&clip) noexcept {
    // move the clips data to ours, resetting the clip's data
    mData = std::exchange(clip.mData, nullptr);
    mLocation = clip.mLocation;
    clip.mLocation = {};
    return *this;
}

PatternClip::~PatternClip() {

}

bool PatternClip::hasData() const {
    return bool(mData);
}

PatternSelection const& PatternClip::selection() {
    return mLocation;
}

void PatternClip::restore(trackerboy::Pattern &dest) const {
    pasteImpl(dest, std::nullopt, false);

}

void PatternClip::paste(trackerboy::Pattern &dest, PatternCursor pos, bool mix) const {
    if (mix) {
        pasteImpl(dest, pos, true);
    } else {
        pasteImpl(dest, pos, false);
    }

}

void PatternClip::pasteImpl(trackerboy::Pattern &dest, std::optional<PatternCursor> pos, bool mixPaste) const {
    

    char const* bufAtRowStart = mData.get();
    auto iter = mLocation.iterator();
    auto const rowLength = TU::getRowLength(iter);
    
    int trackEnd, rowEnd, rowStart;

    if (pos) {
        auto destRegion = mLocation;
        destRegion.moveTo(*pos);

        iter = destRegion.iterator();

        auto clipRowOffset = 0;
        rowStart = iter.rowStart();
        if (rowStart < 0) {
            clipRowOffset = -rowStart;
            rowStart = 0;
        }
        rowEnd = std::min((int)dest.size() - 1, iter.rowEnd());
        trackEnd = std::min(iter.trackEnd(), PatternCursor::MAX_TRACKS - 1);

        Q_ASSERT(rowStart <= rowEnd);

        bufAtRowStart += (rowLength * clipRowOffset);
    } else {
        // no position to paste at given, paste at the source of the clip
        trackEnd = iter.trackEnd();
        rowEnd = iter.rowEnd();
        rowStart = iter.rowStart();
    }

    for (auto track = iter.trackStart(); track <= trackEnd; ++track) {
        auto const tmeta = iter.getTrackMeta(track);
        auto const offset = TU::columnToOffset(tmeta.columnStart());
        auto const length = TU::columnToLength(tmeta.columnEnd()) - offset;
        // with this assertion passing, we will never read or write past
        // the bounds of a TrackRow
        Q_ASSERT(offset + length <= sizeof(trackerboy::TrackRow));
        Q_ASSERT(length > 0);


        if (track >= 0) {
            auto bufAtTrack = bufAtRowStart;
            for (int row = rowStart; row <= rowEnd; ++row) {
                auto &rowdata = dest.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                if (mixPaste) {
                    // construct a TrackRow from the clip data
                    trackerboy::TrackRow src;
                    std::copy_n(bufAtTrack, length, reinterpret_cast<char*>(&src) + offset);
                    // Note: TrackRow fields are 0 for no setting
                    // mix paste, if the destination column is 0 then paste from src's
                    if (tmeta.hasColumn<PatternAnchor::SelectNote>()) {
                        if (rowdata.note == 0 && src.note != 0) {
                            rowdata.note = src.note;
                        }
                    }
                    if (tmeta.hasColumn<PatternAnchor::SelectInstrument>()) {
                        if (rowdata.instrumentId == 0 && src.instrumentId != 0) {
                            rowdata.instrumentId = src.instrumentId;
                        }
                    }
                    for (auto effectNo = tmeta.effectStart(); effectNo < tmeta.effectEnd(); ++effectNo) {
                        auto srcEffect = src.queryEffect(effectNo);
                        auto destEffect = rowdata.queryEffect(effectNo);
                        if (!destEffect && srcEffect) {
                            rowdata.effects[effectNo] = *srcEffect;
                        }
                    }
                } else {
                    // overwrite paste, copy clip data to the pattern
                    std::copy_n(bufAtTrack, length, reinterpret_cast<char*>(&rowdata) + offset);
                }

                // advance to the next row
                bufAtTrack += rowLength;
            }
        }

        // advance to the next track
        bufAtRowStart += length;
    }
}

void PatternClip::save(trackerboy::Pattern const& src, PatternSelection region) {

    // before we do anything, check that the selection is within the bounds
    // of the given pattern
    #ifndef QT_NO_DEBUG
    {
        auto start = region.start();
        auto end = region.end();

        Q_ASSERT(start.isValid());
        Q_ASSERT(end.isValid());
        Q_ASSERT(start.row < src.size());
        Q_ASSERT(end.row < src.size());
    }
    #endif


    mLocation = region;
    auto const iter = mLocation.iterator();

    // determine the row length
    auto const rowLength = TU::getRowLength(iter);

    auto const bufsize = rowLength * iter.rows();
    Q_ASSERT(bufsize != 0);
    mData = std::make_unique<char[]>(bufsize);

    auto bufAtRowStart = mData.get();
    for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
        auto const tmeta = iter.getTrackMeta(track);
        auto const offset = TU::columnToOffset(tmeta.columnStart());
        auto const length = TU::columnToLength(tmeta.columnEnd()) - offset;
        // with this assertion passing, we will never read or write past
        // the bounds of a TrackRow
        Q_ASSERT(offset + length <= sizeof(trackerboy::TrackRow));
        Q_ASSERT(length > 0); // if the length is 0 we aren't copying anything, consider this an error

        auto bufAtTrackStart = bufAtRowStart;
        for (int row = iter.rowStart(); row <= iter.rowEnd(); ++row) {
            auto &rowdata = src.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
            std::copy_n(reinterpret_cast<const char*>(&rowdata) + offset, length, bufAtTrackStart);
            // advance to next row
            bufAtTrackStart += rowLength;
        }

        // advance to next track
        bufAtRowStart += length;
    }
}

void PatternClip::toMime(QMimeData *mime) const {

    auto iter = mLocation.iterator();
    size_t datasize = TU::getRowLength(iter) * iter.rows();
    QByteArray arr((int)(sizeof(mLocation) + datasize), '\0');

    auto dataptr = arr.data();
    std::copy_n(reinterpret_cast<const char*>(&mLocation), sizeof(mLocation), dataptr);
    dataptr += sizeof(mLocation);
    std::copy_n(mData.get(), datasize, dataptr);

    mime->setData(MIME_TYPE, arr);
}

bool PatternClip::fromMime(QMimeData const* mime) {
    if (!mime->hasFormat(MIME_TYPE)) {
        // invalid, not the right format
        return false;
    }

    auto const arr = mime->data(MIME_TYPE);
    auto size = (size_t)arr.size();
    if (size <= sizeof(mLocation)) {
        // not enough data to store the clip's location, mime data is invalid
        return false;
    }
    // remainder of the byte array is the clip data
    size -= sizeof(mLocation);

    auto dataptr = arr.data();
    std::copy_n(dataptr, sizeof(mLocation), reinterpret_cast<char*>(&mLocation));
    dataptr += sizeof(mLocation);

    auto iter = mLocation.iterator();
    size_t datasize = TU::getRowLength(iter) * iter.rows();
    if (datasize == size) {
        mData = std::make_unique<char[]>(datasize);
        std::copy_n(dataptr, datasize, mData.get());
        return true;
    } else {
        // the clipped data buffer size does not match the size of the clip!
        return false;
    }

}

bool operator==(PatternClip const& lhs, PatternClip const& rhs) noexcept {
    if (lhs.mData && rhs.mData) {
        auto leftIter = lhs.mLocation.iterator();
        auto rightIter = rhs.mLocation.iterator();

        // check if the selection is the same
        if (leftIter.start() == rightIter.start() && leftIter.end() == rightIter.end()) {
            auto leftsize = TU::getRowLength(leftIter) * leftIter.rows();
            auto rightsize = TU::getRowLength(rightIter) * rightIter.rows();
            if (leftsize != rightsize) {
                return false;
            } else {
                // determine if the clipped data is the same
                return std::equal(lhs.mData.get(), lhs.mData.get() + leftsize, rhs.mData.get());
            }
        } else {
            return false;
        }
    }

    return lhs.mData == nullptr && rhs.mData == nullptr;
}

bool operator!=(PatternClip const& lhs, PatternClip const& rhs) noexcept {
    return !(lhs == rhs);
}

#undef TU
