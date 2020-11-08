
#include "trackerboy/engine/RuntimeContext.hpp"

namespace trackerboy {


RuntimeContext::RuntimeContext(gbapu::Apu &apu, InstrumentTable &instTable, WaveTable &waveTable) :
    apu(apu),
    instTable(instTable),
    waveTable(waveTable)
{
}


}
