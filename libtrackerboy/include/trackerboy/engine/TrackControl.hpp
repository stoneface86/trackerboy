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

#include "trackerboy/trackerboy.hpp"
#include "trackerboy/data/TrackRow.hpp"
#include "trackerboy/engine/Operation.hpp"
#include "trackerboy/engine/ChannelState.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/GlobalState.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"


namespace trackerboy {

class TrackControl {

public:
    TrackControl(ChType ch, FrequencyControl &fc);

    void setRow(TrackRow const& row);

    void step(RuntimeContext const& rc, ChannelState &state, GlobalState &global);

private:


    Operation mOp;

    std::shared_ptr<Instrument> mInstrument;
    FrequencyControl &mFc;
    std::optional<InstrumentRuntime> mIr;

    std::optional<uint8_t> mDelayCounter;
    std::optional<uint8_t> mCutCounter;

    bool mPlaying;

    uint8_t mEnvelope;
    uint8_t mPanning;
    uint8_t mTimbre;


};

class ToneTrackControl : public TrackControl {

public:
    ToneTrackControl(ChType ch);

private:
    ToneFrequencyControl mFc;


};

class NoiseTrackControl : public TrackControl {

public:
    NoiseTrackControl();

private:
    NoiseFrequencyControl mFc;


};


}
