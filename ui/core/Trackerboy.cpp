
#include "core/Trackerboy.hpp"


Trackerboy::Trackerboy() :
    miniaudio(),
    config(miniaudio),
    document(),
    instrumentModel(document),
    orderModel(document),
    songModel(document, orderModel),
    waveModel(document),
    renderer(
        miniaudio,
        document,
        instrumentModel,
        songModel,
        waveModel
    )
{
}
