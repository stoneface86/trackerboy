
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
