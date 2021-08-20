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
