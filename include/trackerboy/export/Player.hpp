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
    using Duration = std::variant<unsigned, std::chrono::seconds>;

    Player(Engine &engine);

    void start(Duration duration);

    //
    // Gets the playing status of the player.
    // true indicates that the player can step. If false is returned calling 
    // step afterwards will do nothing.
    //
    bool isPlaying() const;

    unsigned progress() const;
    unsigned progressMax() const;

    //
    // Steps the engine, returns true if the player has another step.
    //
    void step();

private:

    struct LoopContext {
        uint8_t currentPattern;
        // keep track of how many times a pattern is visited
        // we have looped the song x times when we have visited a pattern x times
        std::vector<unsigned> visits;
        unsigned loopAmount;

        LoopContext(unsigned loopAmount);
    };

    struct DurationContext {
        unsigned frameCounter;
        unsigned framesToPlay;

        DurationContext(unsigned framesToPlay);
    };

    using ContextVariant = std::variant<std::monostate, LoopContext, DurationContext>;


    Engine &mEngine;
    Engine::Frame mLastFrame;
    bool mPlaying;
    ContextVariant mContext;


};

}
