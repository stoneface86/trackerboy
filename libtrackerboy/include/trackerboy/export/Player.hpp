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

#include "trackerboy/engine/Engine.hpp"

#include <chrono>
#include <variant>
#include <vector>

namespace trackerboy {

//
// Player class for exporting music. Steps the engine so that the song is
// looped a specified number of times or plays for a specified duration.
//
class Player {

public:
    using Duration = std::variant<int, std::chrono::seconds>;

    Player(Engine &engine);

    void start(Duration duration);

    //
    // Gets the playing status of the player.
    // true indicates that the player can step. If false is returned calling 
    // step afterwards will do nothing.
    //
    bool isPlaying() const;

    int progress() const;
    int progressMax() const;

    //
    // Steps the engine, returns true if the player has another step.
    //
    void step();

private:

    struct LoopContext {
        int currentPattern;
        // keep track of how many times a pattern is visited
        // we have looped the song x times when we have visited a pattern x times
        std::vector<int> visits;
        int loopAmount;

        LoopContext(int loopAmount);
    };

    struct DurationContext {
        int frameCounter;
        int framesToPlay;

        DurationContext(int framesToPlay);
    };

    using ContextVariant = std::variant<std::monostate, LoopContext, DurationContext>;


    Engine &mEngine;
    Frame mLastFrame;
    bool mPlaying;
    ContextVariant mContext;


};

}
