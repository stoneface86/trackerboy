
#include "audio.hpp"


namespace audio {

PaException::PaException(PaError err) :
    err(err),
    std::runtime_error("A PortAudio error occurred")
{
}


PaError PaException::getError() {
    return err;
}



}
