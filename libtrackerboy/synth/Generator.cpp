
#include "trackerboy/synth/Generator.hpp"


namespace trackerboy {

Generator::Generator(uint32_t defaultPeriod, uint8_t defaultOutput) noexcept :
    mFreqCounter(0),
    mPeriod(defaultPeriod),
    mOutput(defaultOutput),
    mDisableMask(ENABLED)
{
}

void Generator::disable() noexcept {
    mDisableMask = DISABLED;
}

bool Generator::disabled() const noexcept {
    return mDisableMask == DISABLED;
}

void Generator::restart() noexcept {
    mFreqCounter = 0;
    mDisableMask = ENABLED;
}


}
