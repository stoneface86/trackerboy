
#include "trackerboy/synth/WaveOsc.hpp"


namespace trackerboy {


WaveOsc::WaveOsc(float samplingRate) :
    Osc(samplingRate, 2, 32)
{
}

void WaveOsc::setWaveform(Waveform &wave) {

    auto wavedata = wave.data();

    // clear existing waveform
    mDeltaBuf.clear();

    // convert waveform to a delta buffer

    size_t bufsize = mWaveformSize / 2;
    // bitmask used to wrap -1 to bufsize-1 or bufsize to 0
    size_t mask = bufsize - 1;
    uint8_t lo; // keep lo out here cause we need the last one for initSample
    size_t waveIndex = 0;

    mInitialVolume = (wavedata[0] >> 4) * VOLUME_STEP - VOLUME_MIN;

    for (size_t i = 0; i != mWaveformSize; ++i) {

        uint8_t hi = wavedata[waveIndex];
        lo = hi & 0xF;
        hi >>= 4;

        int8_t delta;
        if ((i & 1) == 1) {
            // for odd numbered indices, the delta is calculated by
            // subtracting the high nibble of the next byte from the low nibble of the current byte
            delta = (wavedata[(waveIndex + 1) & mask] >> 4) - lo;
            ++waveIndex;
        } else {
            // even numbered indices, the delta is the lower nibble
            // minus the high nibble of the current byte
            delta = lo - hi;
        }

        if (delta) {
            Delta d;
            d.change = VOLUME_STEP * delta;
            d.location = static_cast<uint8_t>(i);
            mDeltaBuf.push_back(d);
        }
    }

    mRegenPeriod = true;
}


}
