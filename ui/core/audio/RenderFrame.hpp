
#pragma once

#include "trackerboy/engine/Frame.hpp"

#include <cstddef>
#include <cstdint>

//
// Structure for a previously renderered frame, for visualizers
//
struct RenderFrame {

    // engine information
    trackerboy::Frame engineFrame;

    // APU register dump
    uint8_t registers[20 + 3];

    size_t nsamples;

    int16_t *data;
};
