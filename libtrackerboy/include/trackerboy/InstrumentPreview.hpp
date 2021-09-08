/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2021 stoneface86
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
#include "trackerboy/engine/IApu.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"


#include <optional>

namespace trackerboy {

//
// Utility class for previewing instruments.
//
class InstrumentPreview {

public:

    InstrumentPreview();

    //
    // Start previewing the given instrument. If instrument is nullptr, then
    // the runtime will preview the set note on the given channel. If no channel
    // is provided, then it will default to the instrument's channel or CH1 if
    // no instrument was provided.
    //
    void setInstrument(std::shared_ptr<Instrument> instrument, std::optional<ChType> ch = std::nullopt);

    void play(uint8_t note);

    void step(RuntimeContext const& rc);

private:

    void restart();

    ChType mCh;
    ToneFrequencyControl mToneFc;
    NoiseFrequencyControl mNoiseFc;
    FrequencyControl *mFc;

    std::shared_ptr<Instrument> mInstrument;
    std::optional<InstrumentRuntime> mIr;

    bool mInit;
    bool mRetrigger;
    ChannelState mLastState;
};


}
