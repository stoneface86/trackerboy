
#pragma once

#include <cstdint>


namespace trackerboy {


class Generator {

public:

    //
    // Disables the generator. A disabled generator always outputs 0
    // Generators can only be re-enabled by restarting.
    //
    void disable();

    //
    // Restart (retrigger) the generator. Counters are reset to 0. If the
    // generator was disabled, it is re-enabled.
    //
    virtual void restart();

    //
    // Returns the fence, or the number of cycles to complete a period.
    //
    inline uint32_t fence() {
        return (mFreqCounter > mPeriod) ? 0 : mPeriod - mFreqCounter;
    }

    //
    // Return the current output of the generator. Channels with an envelope
    // return 1 for the current envelope value and 0 for off.
    //
    inline uint8_t output() {
        return mDisableMask & mOutput;
    }


protected:

    Generator(uint32_t defaultPeriod, uint8_t defaultOutput);


    uint32_t mFreqCounter;
    uint32_t mPeriod;

    uint8_t mOutput;

private:

    static constexpr uint8_t ENABLED  = 0xFF;
    static constexpr uint8_t DISABLED = 0x00;

    uint8_t mDisableMask;

};



}
