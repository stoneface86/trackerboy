
#include "trackerboy/engine/RuntimeContext.hpp"

namespace trackerboy {


RuntimeContext::RuntimeContext(IApu &apu, InstrumentTable &instrumentTable, WaveformTable &waveTable) :
    apu(apu),
    instrumentTable(instrumentTable),
    waveTable(waveTable)
{
}


}
