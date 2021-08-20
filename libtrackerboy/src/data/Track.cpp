
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
constexpr Effect NULL_EFFECT = { EffectType::noEffect, 0 };

}

Track::Track(int rows) :
    mData(rows)
{
}

TrackRow& Track::operator[](int row) {
    return mData[row];
}

TrackRow const& Track::operator[](int row) const {
    return mData[row];
}

Track::Data::iterator Track::begin() {
    return mData.begin();
}

Track::Data::const_iterator Track::begin() const {
    return mData.begin();
}

void Track::clear(int rowStart, int rowEnd) {

    int size = std::min(static_cast<int>(mData.size()), rowEnd);
    auto iter = mData.begin() + rowStart;
    for (int i = rowStart; i < size; ++i) {
        *iter++ = NULL_ROW;
    }
}

void Track::clearEffect(int rowNo, int effectNo) {
    assert(effectNo < TrackRow::MAX_EFFECTS);

    auto &row = mData[rowNo];
    row.effects[effectNo] = NULL_EFFECT;

}

void Track::clearInstrument(int rowNo) {
    auto &row = mData[rowNo];
    row.setInstrument({});

}

void Track::clearNote(int rowNo) {
    auto &row = mData[rowNo];
    row.setNote({});
}

Track::Data::iterator Track::end() {
    return mData.end();
}

Track::Data::const_iterator Track::end() const {
    return mData.end();
}

void Track::setEffect(int rowNo, int effectNo, EffectType effect, uint8_t param) {
    assert(effectNo < TrackRow::MAX_EFFECTS);

    if (effect == EffectType::noEffect) {
        clearEffect(rowNo, effectNo);
        return;
    }

    auto &row = mData[rowNo];
    auto &effectSt = row.effects[effectNo];
    effectSt.type = effect;
    effectSt.param = param;
}

void Track::setInstrument(int rowNo, uint8_t instrumentId) {
    auto &row = mData[rowNo];
    row.setInstrument(instrumentId);
}

void Track::setNote(int rowNo, uint8_t note) {
    auto &row = mData[rowNo];
    row.setNote(note);
}

void Track::replace(int rowNo, TrackRow &row) {
    // TODO: this function is now useless, remove it
    mData[rowNo] = row;
}

void Track::resize(int newSize) {
    mData.resize(newSize);
}

int Track::rowCount() const {
    int count = 0;
    for (auto &row : mData) {
        if (!row.isEmpty()) {
            ++count;
        }
    }
    return count;
}

int Track::size() const {
    return (int)mData.size();
}


}
