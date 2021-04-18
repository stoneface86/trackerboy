
#include "core/Trackerboy.hpp"


Trackerboy::Trackerboy() :
    spinlock(),
    miniaudio(),
    config(miniaudio),
    document(spinlock),
    instrumentModel(document),
    orderModel(document),
    waveModel(document),
    renderer(
        miniaudio,
        spinlock,
        document
    )
{
}
