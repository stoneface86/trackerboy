
#pragma once

#include "trackerboy/data/Song.hpp"
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


class MusicRuntime {

public:
    MusicRuntime(Song &song, uint8_t orderNo, uint8_t patternRow);

    void halt();

    void lock(RuntimeContext const& rc, ChType ch);

    void unlock(RuntimeContext const& rc, ChType ch);

    void step(RuntimeContext const& rc);

private:

    template <ChType ch = ChType::ch1>
    void update(RuntimeContext const& rc);

    static constexpr size_t FLAG_LOCK1 = 0;
    static constexpr size_t FLAG_LOCK2 = 1;
    static constexpr size_t FLAG_LOCK3 = 2;
    static constexpr size_t FLAG_LOCK4 = 3;
    static constexpr size_t FLAG_HALT = 4;
    static constexpr size_t FLAG_INIT = 5;

    static constexpr size_t DEFAULT_FLAGS = 1 << FLAG_INIT;

    Song &mSong;

    uint8_t mOrderCounter;
    uint8_t mRowCounter;

    //uint8_t const mLastOrder;
    //uint8_t const mRowsPerTrack;

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
