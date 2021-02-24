
#include "core/Trackerboy.hpp"


Trackerboy::Trackerboy() :
    spinlock(),
    miniaudio(),
    config(miniaudio),
    document(spinlock),
    instrumentModel(document),
    orderModel(document),
    songModel(document, orderModel),
    waveModel(document),
    renderer(
        miniaudio,
        spinlock,
        document,
        instrumentModel,
        songModel,
        waveModel
    )
{
}
