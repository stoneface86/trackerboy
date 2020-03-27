
#include "trackerboy/synth/Generator.hpp"


namespace trackerboy {

Generator::Generator(uint32_t defaultPeriod, uint8_t defaultOutput) :
    mFreqCounter(0),
    mPeriod(defaultPeriod),
    mOutput(defaultOutput),
    mDisableMask(ENABLED)
{
}

void Generator::disable() {
    mDisableMask = DISABLED;
}

void Generator::restart() {
    mFreqCounter = 0;
    mDisableMask = ENABLED;
}


}
