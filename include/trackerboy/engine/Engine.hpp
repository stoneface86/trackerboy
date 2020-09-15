
#pragma once

#include <cstdint>
#include <optional>
#include <tuple>

#include "trackerboy/engine/ChannelControl.hpp"
#include "trackerboy/engine/MusicRuntime.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"
#include "trackerboy/data/Song.hpp"


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
