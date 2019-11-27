
#include "trackerboy/pattern/Track.hpp"


namespace trackerboy {

using Iterator = Track::Iterator;

#define rowptr(row) (mBegin + (static_cast<size_t>(row) * 4))


Track::Track(TrackRow *begin, TrackRow *end) :
    mBegin(begin),
    mEnd(end) {
}

Iterator Track::begin() {
    return Iterator(mBegin);
}

void Track::clear(uint8_t rowStart, uint8_t rowEnd) {
    TrackRow *row = rowptr(rowStart);
    unsigned count = rowEnd - rowStart;
    while (row < mEnd && count--) {
        *row = { 0 };
        row += 4;
    }
}

void Track::clearEffect(uint8_t row) {
    TrackRow *rowdata = rowptr(row);
    rowdata->flags &= ~TrackRow::COLUMN_EFFECT;
}

void Track::clearInstrument(uint8_t row) {
    TrackRow *rowdata = rowptr(row);
    rowdata->flags &= ~TrackRow::COLUMN_INST;
}

void Track::clearNote(uint8_t row) {
    TrackRow *rowdata = rowptr(row);
    rowdata->flags &= ~TrackRow::COLUMN_NOTE;
}

Iterator Track::end() {
    return Iterator(mEnd);
}

void Track::setEffect(uint8_t row, EffectType effect, uint8_t param) {
    TrackRow *rowdata = rowptr(row);
    uint8_t flags = rowdata->flags & 0xF; // discard old effect
    // update column flags so that we know an effect has been set
    rowdata->flags = flags | static_cast<uint8_t>(effect) | TrackRow::COLUMN_EFFECT;
    rowdata->effectParameter = param;

}

void Track::setInstrument(uint8_t row, uint8_t instrumentId) {
    TrackRow *rowdata = rowptr(row);
    rowdata->instrumentId = instrumentId;
    rowdata->flags |= TrackRow::COLUMN_INST;
}

void Track::setNote(uint8_t row, uint8_t note) {
    TrackRow *rowdata = rowptr(row);
    rowdata->note = note;
    rowdata->flags |= TrackRow::COLUMN_NOTE;
}

Iterator::Iterator(Iterator::pointer ptr) :
    mPtr(ptr) {
}

Iterator::Iterator(const Iterator::self_type &iter) :
    mPtr(iter.mPtr) {
}

Iterator::~Iterator() {
}

Iterator::self_type Iterator::operator++() {
    mPtr += 4; // each row in the pattern has 4 track rows
    return *this;
}

Iterator::self_type Iterator::operator++(int junk) {
    (void)junk;
    Iterator::self_type *i = this;
    mPtr += 4;
    return *i;
}

const Iterator::reference Iterator::operator*() {
    return *mPtr;
}

const Iterator::pointer Iterator::operator->() {
    return mPtr;
}

bool Iterator::operator==(const Iterator::self_type &rhs) {
    return mPtr == rhs.mPtr;
}

bool Iterator::operator!=(const Iterator::self_type &rhs) {
    return mPtr != rhs.mPtr;
}

Iterator::self_type Iterator::operator=(const Iterator::self_type &rhs) {
    mPtr = rhs.mPtr;
    return *this;
}



}