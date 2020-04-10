
#include "trackerboy/pattern/Track.hpp"

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

Track::Track(TrackData::iterator begin, TrackData::iterator end) :
    mBegin(begin),
    mEnd(end)
{
    assert(begin < end);
}

TrackData::iterator Track::begin() {
    return mBegin;
}

void Track::clear(uint8_t rowStart, uint8_t rowEnd) {

    auto end = mBegin + rowEnd;
    if (end > mEnd) {
        end = mEnd;
    }

    for (auto iter = mBegin + rowStart; iter < end; ++iter) {
        *iter = NULL_ROW;
    }
}

void Track::clearEffect(uint8_t row, uint8_t effectNo) {
    assert(effectNo < TrackRow::MAX_EFFECTS);
    auto rowdata = mBegin + row;
    assert(rowdata < mEnd);
    rowdata->flags &= ~(TrackRow::COLUMN_EFFECT1 << effectNo);
    rowdata->effects[effectNo] = NULL_EFFECT; // Warning C6386
}

void Track::clearInstrument(uint8_t row) {
    auto rowdata = mBegin + row;
    assert(rowdata < mEnd);
    rowdata->flags &= ~TrackRow::COLUMN_INST;
    rowdata->instrumentId = 0;
}

void Track::clearNote(uint8_t row) {
    auto rowdata = mBegin + row;
    assert(rowdata < mEnd);
    rowdata->flags &= ~TrackRow::COLUMN_NOTE;
    rowdata->note = 0;
}

TrackData::iterator Track::end() {
    return mEnd;
}

void Track::setEffect(uint8_t row, uint8_t effectNo, EffectType effect, uint8_t param) {
    auto rowdata = mBegin + row;
    assert(rowdata < mEnd);
    assert(effectNo < TrackRow::MAX_EFFECTS);

    Effect &effectInRow = rowdata->effects[effectNo]; // Warning C6385

    effectInRow.type = effect;
    effectInRow.param = param;

    // update column flags so that we know an effect has been set
    rowdata->flags |= (TrackRow::COLUMN_EFFECT1 << effectNo);
    

}

void Track::setInstrument(uint8_t row, uint8_t instrumentId) {
    auto rowdata = mBegin + row;
    assert(rowdata < mEnd);
    rowdata->instrumentId = instrumentId;
    rowdata->flags |= TrackRow::COLUMN_INST;
}

void Track::setNote(uint8_t row, uint8_t note) {
    auto rowdata = mBegin + row;
    assert(rowdata < mEnd);
    rowdata->note = note;
    rowdata->flags |= TrackRow::COLUMN_NOTE;
}



}
