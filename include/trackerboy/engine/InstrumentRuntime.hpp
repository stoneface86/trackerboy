/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

#pragma once

#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/NoteControl.hpp"
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
