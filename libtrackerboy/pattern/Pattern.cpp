
#include "trackerboy/pattern/Pattern.hpp"

#define calcIndex(trackId, row) ((row * 4) + static_cast<uint8_t>(trackId))


namespace trackerboy {

using TrackIterator = Pattern::TrackIterator;

Pattern::Pattern(size_t nrows) :
    mSize(nrows)
{
    mData.resize(mSize * 4);
}

void Pattern::clearNote(ChType trackId, uint8_t row) {
    mData[calcIndex(trackId, row)].flags &= ~TrackRow::COLUMN_NOTE;
}

void Pattern::clearInstrument(ChType trackId, uint8_t row) {
    mData[calcIndex(trackId, row)].flags &= ~TrackRow::COLUMN_INST;
}

void Pattern::clearEffect(ChType trackId, uint8_t row) {
    mData[calcIndex(trackId, row)].flags &= ~TrackRow::COLUMN_EFFECT;
}

void Pattern::fromStream(std::vector<uint8_t> &streamBuf) {
    (void)streamBuf;
    // TODO: load pattern data from given pattern stream (model <- file)
}

void Pattern::setNote(ChType trackId, uint8_t row, Note note) {
    TrackRow &rowdata = mData[calcIndex(trackId, row)];
    rowdata.note = note;
    // update column flags so that we know a note has been set
    rowdata.flags |= TrackRow::COLUMN_NOTE;
}

void Pattern::setInstrument(ChType trackId, uint8_t row, uint8_t instrumentId) {
    TrackRow &rowdata = mData[calcIndex(trackId, row)];
    rowdata.instrumentId = instrumentId;
    // update column flags so that we know an instrument has been set
    rowdata.flags |= TrackRow::COLUMN_INST;
}

void Pattern::setEffect(ChType trackId, uint8_t row, EffectType effect, uint8_t param) {
    TrackRow &rowdata = mData[calcIndex(trackId, row)];
    // set the note set bit (indicates a note was set in the row)
    uint8_t flags = rowdata.flags & 0xF; // discard old effect
    // update column flags so that we know an effect has been set
    rowdata.flags = flags | static_cast<uint8_t>(effect) | TrackRow::COLUMN_EFFECT;
    rowdata.effectParameter = param;
}

void Pattern::setSize(size_t nrows) {
    mSize = nrows;
    mData.resize(nrows * 4);
}

size_t Pattern::size() {
    return mSize;
}

void Pattern::toStream(std::vector<uint8_t> &streamBuf) {
    // TODO: convert mData to a pattern stream (model -> file)
    (void)streamBuf;
}

TrackIterator Pattern::trackBegin(ChType trackId) {
    return TrackIterator(mData.data() + static_cast<uint8_t>(trackId));
}

TrackIterator Pattern::trackEnd(ChType trackId) {
    return TrackIterator(mData.data() + mData.size() + static_cast<uint8_t>(trackId));
}

//
// TrackIterator class
//

TrackIterator::TrackIterator(TrackIterator::pointer ptr) :
    mPtr(ptr)
{
}

TrackIterator::TrackIterator(const TrackIterator::self_type &iter) :
    mPtr(iter.mPtr) 
{    
}

TrackIterator::~TrackIterator() {
}

TrackIterator::self_type TrackIterator::operator++() {
    mPtr += 4; // each row in the pattern has 4 track rows
    return *this;
}

TrackIterator::self_type TrackIterator::operator++(int junk) {
    (void)junk;
    TrackIterator::self_type *i = this;
    mPtr += 4;
    return *i;
}

const TrackIterator::reference TrackIterator::operator*() {
    return *mPtr;
}

const TrackIterator::pointer TrackIterator::operator->() {
    return mPtr;
}

bool TrackIterator::operator==(const TrackIterator::self_type &rhs) {
    return mPtr == rhs.mPtr;
}

bool TrackIterator::operator!=(const TrackIterator::self_type &rhs) {
    return mPtr != rhs.mPtr;
}

TrackIterator::self_type TrackIterator::operator=(const TrackIterator::self_type &rhs) {
    mPtr = rhs.mPtr;
    return *this;
}

}