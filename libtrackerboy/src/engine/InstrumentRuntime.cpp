
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/note.hpp"
#include "internal/enumutils.hpp"

namespace trackerboy {


InstrumentRuntime::InstrumentRuntime(Instrument const& instrument) :
    mEnvelope(instrument.queryEnvelope()),
    mTimbreSequence(instrument.enumerateSequence(Instrument::SEQUENCE_TIMBRE)),
    mPanningSequence(instrument.enumerateSequence(Instrument::SEQUENCE_PANNING))
{
}


void InstrumentRuntime::step(ChannelState &state) {

    if (mEnvelope) {
        state.envelope = *mEnvelope;
        mEnvelope.reset();
    }

    // run the sequences
    auto panning = mPanningSequence.next();
    if (panning) {
        state.panning = *panning;
    }

    auto timbre = mTimbreSequence.next();
    if (timbre) {
        state.timbre = *timbre;
    }
    

}

}
