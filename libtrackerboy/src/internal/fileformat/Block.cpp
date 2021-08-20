
#include "internal/fileformat/Block.hpp"

#include "internal/endian.hpp"


namespace trackerboy {

InputBlock::InputBlock(std::istream &stream) :
    mStream(stream),
    mSize(0),
    mPosition(0)
{

}

size_t InputBlock::size() const {
    return mSize;
}

BlockId InputBlock::begin() {
    // read the block type
    BlockId id;
    mStream.read(reinterpret_cast<char*>(&id), sizeof(id));
    if (!mStream.good()) {
        return 0;
    }

    BlockSize size;
    mStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    if (!mStream.good()) {
        return 0;
    }
    mSize = correctEndian(size);
    mPosition = 0;

    return correctEndian(id);
}

//
// Returns true if the entire block has been read
//
bool InputBlock::finished() {
    return mPosition == mSize;
}

template <>
void InputBlock::read(size_t count, char *data) {
    auto newpos = mPosition + count;
    if (newpos > mSize) {
        // attempted to read past the block, error!
        // the data is corrupted or ill-formed
        throw BoundsError();
    }

    mStream.read(data, count);
    mPosition = newpos;
}


OutputBlock::OutputBlock(std::ostream &stream) :
    mStream(stream),
    mLengthPos(0),
    mSize(0)
{

}

void OutputBlock::begin(BlockId id) {
    BlockId idOut = correctEndian(id);
    mStream.write(reinterpret_cast<const char*>(&idOut), sizeof(idOut));

    mLengthPos = mStream.tellp();

    uint32_t size = 0;
    mStream.write(reinterpret_cast<const char*>(&size), sizeof(size));

    mSize = 0;
}

//
// Finish writing the block. The block's length field is updated with
// this block's length.
//
void OutputBlock::finish() {
    if (mSize) {
        auto oldpos = mStream.tellp();
        mStream.seekp(mLengthPos);
        BlockSize size = correctEndian((uint32_t)mSize);
        mStream.write(reinterpret_cast<const char*>(&size), sizeof(size));

        mStream.seekp(oldpos);
    }
}

template <>
void OutputBlock::write(size_t count, const char* data) {
    mStream.write(data, count);
    mSize += count;
}


}
