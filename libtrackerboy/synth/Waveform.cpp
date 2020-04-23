
#include "trackerboy/synth/Waveform.hpp"


namespace trackerboy {


Waveform::Waveform() noexcept :
    mData{0}
{       
}

uint8_t* Waveform::data() noexcept {
    return mData;
}


}
