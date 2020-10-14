
#include "audio.hpp"


namespace audio {

SoundIoError::SoundIoError(int error) noexcept :
    std::runtime_error(soundio_strerror(error)),
    mError(error)
{
}

int SoundIoError::error() const noexcept {
    return mError;
}

}
