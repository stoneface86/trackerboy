
#include "trackerboy/song/Order.hpp"
#include "trackerboy/fileformat.hpp"

#include <stdexcept>


namespace trackerboy {


Order::Order() :
    mLoops(false),
    mLoopIndex(0)
{
}

std::vector<uint8_t>& Order::indexVec() {
    return mIndexVec;
}

uint8_t Order::loopIndex() {
    return (mLoops) ? mLoopIndex : 0;
}

bool Order::loops() {
    return mLoops;
}

void Order::setLoop(uint8_t index) {
    if (index >= mIndexVec.size()) {
        throw std::out_of_range("loop index outside of order");
    }

    mLoopIndex = index;
    mLoops = true;
}

void Order::removeLoop() {
    mLoops = false;
}

Order::Sequencer::Sequencer(Order &order) :
    mLoopCounter(INFINITE_LOOP)
{
    init(order);
}

Order::Sequencer::Sequencer(Order &order, size_t loopFor) :
    mLoopCounter(loopFor)
{
    init(order);
}

void Order::Sequencer::init(Order &order) {
    mPtr = order.mIndexVec.data();
    mEnd = mPtr + order.mIndexVec.size();
    mLoop = (order.mLoops) ? mPtr + order.mLoopIndex : nullptr;
}

bool Order::Sequencer::hasNext() {
    return mPtr != mEnd;
}

uint8_t Order::Sequencer::next() {
    uint8_t val = *mPtr++;

    // loop if a loop point was set and the counter is not 0
    if (mLoop != nullptr && mLoopCounter) {
        if (mPtr == mEnd) {
            mPtr = mLoop;
            if (mLoopCounter != INFINITE_LOOP) {
                --mLoopCounter;
            }
        }
    }

    return val;
}


}