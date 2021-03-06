
#include "trackerboy/data/Sequence.hpp"

#include <stdexcept>

namespace trackerboy {

Sequence::Sequence() :
    mData(),
    mLoop()
{
}

Sequence::Sequence(Sequence const& seq) :
    mData(seq.mData),
    mLoop(seq.mLoop)
{
}

std::vector<uint8_t>& Sequence::data() noexcept {
    return mData;
}

std::vector<uint8_t> const& Sequence::data() const noexcept {
    return mData;
}

void Sequence::resize(size_t size) {
    if (size > MAX_SIZE) {
        throw std::invalid_argument("size must be less than or equal to 256");
    }
    mData.resize(size);
    if (mLoop && *mLoop >= size) {
        mLoop.reset();
    }
}

std::optional<uint8_t> Sequence::loop() const noexcept {
    return mLoop;
}

void Sequence::setLoop(uint8_t loop) {
    mLoop = loop;
}

void Sequence::removeLoop() {
    mLoop.reset();
}


Sequence::Enumerator Sequence::enumerator() const {
    return { *this };
}

Sequence::Enumerator::Enumerator(Sequence const& seq) :
    mSequence(seq),
    mIndex(0)
{
}

bool Sequence::Enumerator::hasNext() {
    // a sequence with a loop always has a next
    return mSequence.mLoop || mIndex < mSequence.mData.size();
}

std::optional<uint8_t> Sequence::Enumerator::next() {

    auto const seqsize = mSequence.mData.size();
    if (mIndex >= seqsize) {
        if (seqsize != 0 && mSequence.mLoop) {
            mIndex = *mSequence.mLoop;
        } else {
            return std::nullopt;
        }
    }

    auto curr = mSequence.mData[mIndex];
    ++mIndex;

    return curr;

}

}


