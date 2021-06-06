
#include "core/clipboard/PatternClip.hpp"

#include <QtGlobal>

#include <algorithm>
#include <type_traits>

constexpr auto MIME_TYPE = "application/trackerboy";

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
    
    size_t length = sizeof(trackerboy::TrackRow) * (trackEnd - trackStart);
    
    length += columnToLength(iter.columnEnd());

    return length - columnToOffset(iter.columnStart());
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

PatternClip::PatternClip(PatternClip &&clip) :
    mLocation(clip.mLocation),
    mData(std::exchange(clip.mData, nullptr))
{
}

PatternClip& PatternClip::operator=(PatternClip const& clip) {
    if (clip.mData) {
        auto iter = clip.mLocation.iterator();
        auto size = getRowLength(iter) * iter.rows();
        mData.reset(new char[size]);
        std::copy_n(clip.mData.get(), size, mData.get());
        mLocation = clip.mLocation;
    } else {
        mData.reset();
        mLocation = PatternSelection();
    }

    return *this;
}

PatternClip& PatternClip::operator=(PatternClip &&clip) {
    std::swap(mData, clip.mData);
    mLocation = clip.mLocation;
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
    pasteImpl<false>(dest, std::nullopt);

}

void PatternClip::paste(trackerboy::Pattern &dest, PatternCursor pos, bool mix) const {
    if (mix) {
        pasteImpl<true>(dest, pos);
    } else {
        pasteImpl<false>(dest, pos);
    }

}

template <bool tMix>
void PatternClip::pasteImpl(trackerboy::Pattern &dest, std::optional<PatternCursor> pos) const {
    

    auto bufAtRowStart = mData.get();
    auto iter = mLocation.iterator();
    auto rowLength = getRowLength(iter);
    
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
        auto tmeta = iter.getTrackMeta(track);
        auto offset = columnToOffset(tmeta.columnStart());
        auto length = columnToLength(tmeta.columnEnd()) - offset;
        // with this assertion passing, we will never read or write past
        // the bounds of a TrackRow
        Q_ASSERT(offset + length <= sizeof(trackerboy::TrackRow));


        if (track >= 0) {
            auto bufAtTrack = bufAtRowStart;
            for (int row = rowStart; row <= rowEnd; ++row) {
                auto &rowdata = dest.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                if constexpr (tMix) {
                    // construct a TrackRow from the clip data
                    trackerboy::TrackRow src;
                    std::copy_n(bufAtTrack, length, reinterpret_cast<char*>(&src) + offset);
                    // Note: TrackRow fields are 0 for no setting
                    // mix paste, if the destination column is 0 then paste from src's
                    if (tmeta.hasColumn<PatternSelection::SelectNote>()) {
                        if (rowdata.note == 0 && src.note != 0) {
                            rowdata.note = src.note;
                        }
                    }
                    if (tmeta.hasColumn<PatternSelection::SelectInstrument>()) {
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
    mLocation = region;
    auto iter = mLocation.iterator();

    // determine the row length
    size_t rowLength = getRowLength(iter);
    mData.reset(new char[rowLength * iter.rows()]);

    auto bufAtRowStart = mData.get();
    for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
        auto tmeta = iter.getTrackMeta(track);
        auto offset = columnToOffset(tmeta.columnStart());
        auto length = columnToLength(tmeta.columnEnd()) - offset;
        // with this assertion passing, we will never read or write past
        // the bounds of a TrackRow
        Q_ASSERT(offset + length <= sizeof(trackerboy::TrackRow));

        auto bufAtTrackStart = bufAtRowStart;
        for (int row = iter.rowStart(); row <= iter.rowEnd(); ++row) {
            auto &rowdata = src.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
            std::copy_n(reinterpret_cast<const char*>(&rowdata) + offset, length, bufAtTrackStart);
            bufAtTrackStart += rowLength;
        }

        // advance to next track
        bufAtRowStart += length;
    }
}

void PatternClip::toMime(QMimeData *mime) const {

    auto iter = mLocation.iterator();
    size_t datasize = getRowLength(iter) * iter.rows();
    QByteArray arr((int)(sizeof(mLocation) + datasize), '\0');

    auto dataptr = arr.data();
    std::copy_n(reinterpret_cast<const char*>(&mLocation), sizeof(mLocation), dataptr);
    dataptr += sizeof(mLocation);
    std::copy_n(mData.get(), datasize, dataptr);

    mime->setData(MIME_TYPE, arr);
}

bool PatternClip::fromMime(QMimeData const* mime) {
    if (!mime->hasFormat(MIME_TYPE)) {
        return false;
    }

    auto const arr = mime->data(MIME_TYPE);
    auto size = (size_t)arr.size();
    if (size <= sizeof(mLocation)) {
        return false;
    }

    auto dataptr = arr.data();
    std::copy_n(dataptr, sizeof(mLocation), reinterpret_cast<char*>(&mLocation));
    dataptr += sizeof(mLocation);

    auto iter = mLocation.iterator();
    size_t datasize = getRowLength(iter) * iter.rows();
    mData.reset(new char[datasize]);

    std::copy_n(dataptr, datasize, mData.get());

    return true;
}


