
#include "trackerboy/engine/RuntimeContext.hpp"

namespace trackerboy {


RuntimeContext::RuntimeContext(IApu &apu, InstrumentList &instList, WaveformList &waveList) :
    apu(apu),
    instList(instList),
    waveList(waveList)
{
}


}
