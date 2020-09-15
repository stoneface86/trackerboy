
#include "trackerboy/engine/ChannelControl.hpp"


namespace trackerboy {

ChannelControl::ChannelControl() :
    mLocks(0)
{
}

bool ChannelControl::isLocked(ChType ch) const noexcept {
    return !(mLocks & (1 << static_cast<int>(ch)));
}

uint8_t ChannelControl::lockbits() const noexcept {
    return mLocks & 0xF;
}

void ChannelControl::lock(ChType ch) noexcept {
    mLocks &= ~(1 << static_cast<int>(ch));
}

void ChannelControl::unlock(ChType ch) noexcept {
    mLocks |= (1 << static_cast<int>(ch));
}




}
