
#pragma once

#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/engine/NoteControl.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"
#include "trackerboy/note.hpp"

namespace trackerboy {

//
// Utility class for previewing an instrument. Not used by the engine.
//
class InstrumentRuntime {

public:
    InstrumentRuntime(RuntimeContext rc);

    void setChannel(ChType ch);

    void setInstrument(Instrument &instrument);

    void playNote(Note note);

    void step();

private:
    RuntimeContext mRc;
    ChType mCh;

    NoteControl mNc;
    FrequencyControl mFc;

    Instrument::Data mInstrument;
    bool mAutoRetrigger;

};



}
