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
// Players are single use objects. If you want to restart playing, create a
// new player.
//
class Player {

public:
    Player(Engine &engine, unsigned loop);
    Player(Engine &engine, std::chrono::seconds seconds);

    //
    // Gets the playing status of the player.
    // true indicates that the player can step. If false is returned calling 
    // step afterwards will do nothing.
    //
    bool isPlaying() const;

    //
    // Steps the engine, returns true if the player has another step.
    //
    void step();

private:
    Player(Engine &engine);

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

    // monostate is there so the variant can be default constructed
    // mContext should never be set to it after construction
    using ContextVariant = std::variant<std::monostate, LoopContext, DurationContext>;


    Engine &mEngine;
    bool mPlaying;
    ContextVariant mContext;


};

}
