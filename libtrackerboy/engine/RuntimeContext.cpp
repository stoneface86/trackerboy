
#include "trackerboy/engine/RuntimeContext.hpp"

namespace trackerboy {


RuntimeContext::RuntimeContext(Synth &synth, InstrumentTable &instTable, WaveTable &waveTable) :
    synth(synth),
    instTable(instTable),
    waveTable(waveTable)
{
}


}
