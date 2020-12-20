
#pragma once

#include "model/ModuleDocument.hpp"
#include "model/InstrumentListModel.hpp"
#include "model/SongListModel.hpp"
#include "model/WaveListModel.hpp"
#include "Config.hpp"
#include "ColorTable.hpp"
#include "Miniaudio.hpp"
#include "Renderer.hpp"

#include <QFont>

//
// Container struct for configuration and model classes used throughout the ui.
//
struct Trackerboy {

    Trackerboy();
    ~Trackerboy() = default;

    Miniaudio miniaudio;
    ColorTable colorTable;
    Config config;

    ModuleDocument document;
    InstrumentListModel instrumentModel;
    SongListModel songModel;
    WaveListModel waveModel;

    Renderer renderer;

};
