
#pragma once

#include <cstdint>


namespace trackerboy {


class Generator {

public:

    //
    // Restart (retrigger) the generator. Counters are reset to 0.
    //
    virtual void restart();

    //
    // Returns the fence, or the number of cycles to complete a period.
    //
    inline uint32_t fence() {
        return mPeriod - mFreqCounter;
    }

    //
    // Return the current output of the generator. Channels with an envelope
    // return 1 for the current envelope value and 0 for off.
    //
    inline uint8_t output() {
        return mOutput;
    }


protected:

    Generator(uint32_t defaultPeriod, uint8_t defaultOutput);


    uint32_t mFreqCounter;
    uint32_t mPeriod;

    uint8_t mOutput;

private:



};



}
