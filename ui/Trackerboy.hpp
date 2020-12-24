
#pragma once

#include "model/ModuleDocument.hpp"
#include "model/InstrumentListModel.hpp"
#include "model/SongListModel.hpp"
#include "model/WaveListModel.hpp"
#include "Config.hpp"
#include "ColorTable.hpp"
#include "Miniaudio.hpp"
#include "Renderer.hpp"

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
    // Spinlock used by the audio callback thread and GUI thread
    // for synchronization
    //
    Spinlock spinlock;

    //
    // The document (module) being worked on
    //
    ModuleDocument document;

    //
    // Model classes provide a read/write interface for the document
    //
    InstrumentListModel instrumentModel;
    OrderModel orderModel;
    SongListModel songModel;
    WaveListModel waveModel;

    //
    // audio renderering + playback
    //
    Renderer renderer;

};
