
#include "Trackerboy.hpp"


Trackerboy::Trackerboy() :
    miniaudio(),
    config(miniaudio),
    document(),
    instrumentModel(document),
    songModel(document),
    waveModel(document),
    renderer(document, instrumentModel, waveModel, config)
{
}
