
#include "core/Trackerboy.hpp"


Trackerboy::Trackerboy() :
    miniaudio(),
    config(miniaudio),
    spinlock(),
    document(spinlock),
    instrumentModel(document),
    orderModel(document),
    songModel(document, orderModel),
    waveModel(document),
    renderer(
        miniaudio,
        spinlock,
        document.instrumentTable(),
        document.waveTable(),
        instrumentModel,
        songModel,
        waveModel
    )
{
}
