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

#include "trackerboy/data/Song.hpp"
#include "trackerboy/engine/ChannelControl.hpp"
#include "trackerboy/engine/MusicRuntime.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"

#include <cstdint>
#include <optional>

namespace trackerboy {


class Engine {

public:

    Engine(RuntimeContext rc);

    void reset();

    void play(Song &song, uint8_t orderNo, uint8_t patternRow = 0);
    
    //
    // Lock the given channel. Channel is reloaded with music settings.
    //
    void lock(ChType ch);

    //
    // Unlock the given channel. Channel is cleared and will no longer be
    // updated by the music runtime.
    //
    void unlock(ChType ch);

    bool step();

private:

    RuntimeContext mRc;
    std::optional<MusicRuntime> mMusicContext;
    ChannelControl mChCtrl;

    // bit 0: CH1 lock status (0 = locked)
    // bit 1: CH2 lock status
    // bit 2: CH3 lock status
    // bit 3: CH4 lock status
    int mChflags;

    //sfx runtime

};


}
