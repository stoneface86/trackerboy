
#pragma once

#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/engine/ChannelState.hpp"


namespace trackerboy {

//
// Runtime class for instruments. Stepping this runtime with an associated
// instrument will modify the given ChannelState according to the Instrument's
// sequence data. Single-use object.
//
class InstrumentRuntime {


public:
    InstrumentRuntime(Instrument const& instrument);

    void step(ChannelState &state);


private:

    std::optional<uint8_t> mEnvelope;

    Sequence::Enumerator mTimbreSequence;
    Sequence::Enumerator mPanningSequence;

    

};


}
