
#pragma once

#include "trackerboy/engine/Frame.hpp"

#include "gbapu.hpp"

#include <cstddef>
#include <cstdint>

//
// Structure for a previously renderered frame, for visualizers
//
struct RenderFrame {

    // engine information
    trackerboy::Frame engineFrame;

    // APU register dump
    gbapu::Registers registers;

};
