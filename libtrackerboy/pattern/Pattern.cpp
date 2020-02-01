
#include "trackerboy/pattern/Pattern.hpp"

#define calcIndex(trackId, row) ((static_cast<size_t>(row) * 4) + static_cast<uint8_t>(trackId))


namespace trackerboy {

Pattern::Pattern(size_t nrows) :
    mSize(nrows)
{
    mData.resize(mSize * 4);
    clear(0, nrows - 1);
}

Pattern::Iterator Pattern::begin() {
    return mData.cbegin();
}


void Pattern::clear(uint8_t rowStart, uint8_t rowEnd) {
    if (rowStart >= mSize || rowEnd >= mSize || rowEnd > rowStart) {
        return; // TODO: throw exception
    }

    TrackRow zeroRow = { 0 };
    size_t offset = static_cast<size_t>(rowStart) * 4;
    do {
        // 4 tracks per row, zero them
        mData[offset++] = zeroRow;
        mData[offset++] = zeroRow;
        mData[offset++] = zeroRow;
        mData[offset++] = zeroRow;
    } while (rowStart++ != rowEnd);
}

TrackRow* Pattern::data() {
    return mData.data();
}

Pattern::Iterator Pattern::end() {
    return mData.cend();
}

void Pattern::setSize(size_t nrows) {
    mSize = nrows;
    mData.resize(nrows * 4);
}

size_t Pattern::size() {
    return mSize;
}

Track Pattern::track(ChType id) {
    TrackRow *begin = mData.data() + static_cast<size_t>(id);
    return Track(begin, begin + (mSize * 4));
}

}