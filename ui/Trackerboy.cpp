
#include "Trackerboy.hpp"


Trackerboy::Trackerboy() :
    miniaudio(),
    config(colorTable, miniaudio),
    document(),
    instrumentModel(document),
    orderModel(document),
    songModel(document, orderModel),
    waveModel(document),
    renderer(document, instrumentModel, waveModel)
{
}
