
#include "trackerboy/engine/RuntimeContext.hpp"

namespace trackerboy {


RuntimeContext::RuntimeContext(IApu &apu, InstrumentTable const& instrumentTable, WaveformTable const& waveTable) :
    apu(apu),
    instrumentTable(instrumentTable),
    waveTable(waveTable)
{
}


}
