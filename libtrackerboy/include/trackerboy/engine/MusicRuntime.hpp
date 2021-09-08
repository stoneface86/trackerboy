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

#include "trackerboy/data/Song.hpp"
#include "trackerboy/engine/Frame.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/engine/GlobalState.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/engine/Timer.hpp"
#include "trackerboy/engine/TrackControl.hpp"

#include <array>
#include <bitset>
#include <cstdint>
#include <optional>

namespace trackerboy {


//
// A MusicRuntime is responsible for playing "music" or pattern data on locked channels.
// The runtime will modify the given IApu each frame via the step() method. The runtime
// plays indefinitely unless it is halted (pattern effect B00). A MusicRuntime can only play
// one song for its entire lifetime.
//
class MusicRuntime {

public:
    MusicRuntime(Song const& song, int orderNo, int patternRow, bool patternRepeat = false);

    void halt(RuntimeContext const& rc);

    void lock(RuntimeContext const& rc, ChType ch);

    void reloadAll(RuntimeContext const& rc);

    void reload(RuntimeContext const& rc, ChType ch);

    void unlock(RuntimeContext const& rc, ChType ch);

    void jump(int pattern);

    bool step(RuntimeContext const& rc, Frame &frame);

    void repeatPattern(bool repeat);

private:

    template <ChType ch = ChType::ch1>
    void update(RuntimeContext const& rc);

    template <ChType ch = ChType::ch1>
    void haltChannels(RuntimeContext const& rc);

    static constexpr size_t FLAG_LOCK1 = 0;
    static constexpr size_t FLAG_LOCK2 = 1;
    static constexpr size_t FLAG_LOCK3 = 2;
    static constexpr size_t FLAG_LOCK4 = 3;
    static constexpr size_t FLAG_HALT = 4;
    static constexpr size_t FLAG_INIT = 5;

    static constexpr size_t DEFAULT_FLAGS = 1 << FLAG_INIT;

    Song const& mSong;

    int mOrderCounter;
    int mRowCounter;

    bool mPatternRepeat;

    Timer mTimer;

    GlobalState mGlobal;

    // flags
    // 0-3: lock status (0: locked, 1: unlocked)
    // 4: halt status
    // 5: init status
    std::bitset<6> mFlags;

    std::array<ChannelState, 4> mStates;

    ToneTrackControl mTc1;
    ToneTrackControl mTc2;
    ToneTrackControl mTc3;
    NoiseTrackControl mTc4;


};


}
