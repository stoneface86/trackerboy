
#include "Trackerboy.hpp"


Trackerboy::Trackerboy() :
    miniaudio(),
    config(miniaudio),
    spinlock(),
    document(spinlock),
    instrumentModel(document),
    orderModel(document),
    songModel(document, orderModel),
    waveModel(document),
    renderer(document, instrumentModel, waveModel)
{
}
