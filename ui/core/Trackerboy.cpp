
#include "core/Trackerboy.hpp"


Trackerboy::Trackerboy() :
    miniaudio(),
    config(miniaudio),
    renderer(
        miniaudio
    )
{
}
