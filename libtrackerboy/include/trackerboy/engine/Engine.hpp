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

#include "trackerboy/data/Module.hpp"
#include "trackerboy/engine/Frame.hpp"
#include "trackerboy/engine/IApu.hpp"
#include "trackerboy/engine/MusicRuntime.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"

#include <cstdint>
#include <optional>

namespace trackerboy {


class Engine {

public:

    Engine(IApu &apu, Module const* mod = nullptr);

    Module const* getModule() const;

    Song const* getSong() const;

    bool canPlay() const;

    void setModule(Module const* mod);

    //
    // Set the song to play music from. Ownership of the song is to be
    // managed by the caller. The engine expects that the lifetime of the
    // song will be the same as the engine's or until reset() is called.
    //
    void setSong(Song const* song);

    void reset();

    //
    // begin playing music from a starting order and row
    //
    void play(int orderNo, int patternRow = 0);

    //
    // Stops music playback if playing music.
    //
    void halt();
    
    void jump(int pattern);

    //
    // Lock the given channel. Channel is reloaded with music settings.
    //
    void lock(ChType ch);

    //
    // Unlock the given channel. Channel is cleared and will no longer be
    // updated by the music runtime.
    //
    void unlock(ChType ch);

    //
    // Writes the current music state to all locked channels.
    //
    void reload();

    //
    // If enabled, the music runtime will keep repeating the current pattern
    //
    void repeatPattern(bool repeat);

    //
    // Step the engine for 1 frame. Details about the frame are written to
    // the given Frame structure.
    //
    void step(Frame &frame);

private:
    void clearChannel(ChType ch);

    IApu &mApu;
    Module const* mModule;
    std::optional<RuntimeContext> mRc;
    std::optional<MusicRuntime> mMusicContext;
    Song const* mSong;

    //TODO: sfx runtime
    int mTime;

    bool mPatternRepeat;

};


}
