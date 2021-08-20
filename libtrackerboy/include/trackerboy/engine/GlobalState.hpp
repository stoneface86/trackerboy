
#pragma once

#include "trackerboy/engine/Operation.hpp"

#include <cstdint>


namespace trackerboy {

//
// Global effect state for the MusicRuntime
//
struct GlobalState {


    constexpr GlobalState() :
        patternCommand(Operation::PatternCommand::none),
        patternCommandParam(0),
        speed(0),
        halt(false)
    {
    }


    // pattern effects / global effects
    Operation::PatternCommand patternCommand;
    uint8_t patternCommandParam;
    uint8_t speed; // change speed if nonzero
    bool halt;



};

}
