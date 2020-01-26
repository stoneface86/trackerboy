
#include "trackerboy/synth/Waveform.hpp"


namespace trackerboy {


Waveform::Waveform() :
    mData{0}
{       
}

uint8_t* Waveform::data() {
    return mData;
}


}