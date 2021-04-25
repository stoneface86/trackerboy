
#include "trackerboy/data/Track.hpp"

#include <algorithm>
#include <cassert>

#ifdef _MSC_VER
// supress the warnings caused by:
// rowdata->effects[effectNo]
// effectNo is checked via assert
// although this would be a very bad buffer overrun to debug
#pragma warning(disable : 6385 6386)
#endif

namespace trackerboy {

namespace {

constexpr TrackRow NULL_ROW = { 0 };
constexpr Effect NULL_EFFECT = { static_cast<EffectType>(0), 0 };

}

Track::Track(uint16_t rows) :
    mRowCounter(0),
    mData(rows)
{
}

TrackRow& Track::operator[](uint16_t row) {
    return mData[row];
}

TrackRow const& Track::operator[](uint16_t row) const {
    return mData[row];
}

Track::Data::iterator Track::begin() {
    return mData.begin();
}

Track::Data::const_iterator Track::begin() const {
    return mData.begin();
}

void Track::clear(uint16_t rowStart, uint16_t rowEnd) {

    uint16_t size = std::min(static_cast<uint16_t>(mData.size()), rowEnd);
    auto iter = mData.begin() + rowStart;
    for (uint16_t i = rowStart; i < size; ++i) {
        if (iter->flags) {
            --mRowCounter; // if this row was set, decrement the counter
        }
        *iter++ = NULL_ROW;
    }
}

void Track::clearEffect(uint8_t rowNo, uint8_t effectNo) {
    assert(effectNo < TrackRow::MAX_EFFECTS);
    uint8_t column = TrackRow::COLUMN_EFFECT1 << effectNo;
    auto &row = updateColumns<true>(rowNo, column);
    row.effects[effectNo] = NULL_EFFECT;
    
}

void Track::clearInstrument(uint8_t rowNo) {
    auto &row = updateColumns<true>(rowNo, TrackRow::COLUMN_INST);
    row.instrumentId = 0;
}

void Track::clearNote(uint8_t rowNo) {
    auto &row = updateColumns<true>(rowNo, TrackRow::COLUMN_NOTE);
    row.note = 0;
}

Track::Data::iterator Track::end() {
    return mData.end();
}

Track::Data::const_iterator Track::end() const {
    return mData.end();
}

void Track::setEffect(uint8_t rowNo, uint8_t effectNo, EffectType effect, uint8_t param) {
    assert(effectNo < TrackRow::MAX_EFFECTS);
    uint8_t column = TrackRow::COLUMN_EFFECT1 << effectNo;
    auto &row = updateColumns<false>(rowNo, column);
    auto &effectSt = row.effects[effectNo];
    effectSt.type = effect;
    effectSt.param = param;
}

void Track::setInstrument(uint8_t rowNo, uint8_t instrumentId) {
    auto &row = updateColumns<false>(rowNo, TrackRow::COLUMN_INST);
    row.instrumentId = instrumentId;
}

void Track::setNote(uint8_t rowNo, uint8_t note) {
    auto &row = updateColumns<false>(rowNo, TrackRow::COLUMN_NOTE);
    row.note = note;
}

void Track::replace(uint8_t rowNo, TrackRow &row) {
    auto &rowToReplace = mData[rowNo];
    if (rowToReplace.flags == 0) {
        // row was empty
        if (row.flags != 0) {
            // non-empty row will replace this one, increment counter
            ++mRowCounter;
        }
    } else {
        // row was non-empty
        if (row.flags == 0) {
            // empty row will replace this one, decrement counter
            --mRowCounter;
        }
    }
    rowToReplace = row;
}

void Track::resize(uint16_t newSize) {
    mData.resize(newSize);
}

uint16_t Track::rowCount() const {
    return mRowCounter;
}


template <bool clear>
TrackRow& Track::updateColumns(uint8_t rowNo, uint8_t columns) {
    assert(rowNo < mData.size());

    auto &row = mData[rowNo];
    uint8_t oldflags = row.flags;

    if constexpr (clear) {
        row.flags &= ~columns;
        if (oldflags != 0 && row.flags == 0) {
            // the row changes from non-empty -> empty, decremented the counter
            --mRowCounter;
        }
    } else {
        row.flags |= columns;
        if (oldflags == 0 && row.flags != 0) {
            // the row changes from empty -> non-empty increment the counter
            ++mRowCounter;
        }
    }

    return row;

}


}
