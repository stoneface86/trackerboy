
#pragma once

#include <cstdint>

#include "trackerboy/ChType.hpp"

namespace trackerboy {

class ChannelControl {

public:
    ChannelControl();

    bool isLocked(ChType ch) const noexcept;

    uint8_t lockbits() const noexcept;

    void lock(ChType ch) noexcept;

    void unlock(ChType ch) noexcept;


private:

    //
    // Bit 0: CH1 Lock status
    // Bit 1: CH2 Lock status
    // Bit 2: CH3 Lock status
    // Bit 3: CH4 Lock status
    // Bit 4: CH1 Sfx trigger status (TODO)
    // Bit 5: CH2 Sfx trigger status
    // Bit 6: CH3 Sfx trigger status
    // Bit 7: CH4 Sfx trigger status
    // 
    //
    uint8_t mLocks;

};

}
