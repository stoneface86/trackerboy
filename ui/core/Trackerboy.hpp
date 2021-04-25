
#pragma once

#include "core/audio/Renderer.hpp"
#include "core/Config.hpp"
#include "core/Miniaudio.hpp"

//
// Container struct for configuration and model classes used throughout the ui.
//
struct Trackerboy {

    Trackerboy();
    ~Trackerboy() = default;

    //
    // manages a miniaudio context and device list
    //
    Miniaudio miniaudio;

    //
    // Application settings
    //
    Config config;

    //
    // audio renderering + playback
    //
    Renderer renderer;

};
