
#pragma once

#include "core/model/ModuleDocument.hpp"
#include "core/model/InstrumentListModel.hpp"
#include "core/model/SongListModel.hpp"
#include "core/model/WaveListModel.hpp"
#include "core/Config.hpp"
#include "core/ColorTable.hpp"
#include "core/Miniaudio.hpp"
#include "core/Renderer.hpp"

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
