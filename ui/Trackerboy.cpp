
#include "Trackerboy.hpp"


Trackerboy::Trackerboy() :
    miniaudio(),
    config(colorTable, miniaudio),
    document(),
    instrumentModel(document),
    songModel(document),
    waveModel(document),
    renderer(document, instrumentModel, waveModel)
{
}
