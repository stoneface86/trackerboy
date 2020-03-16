
#pragma once

#include <ostream>
#include <cstddef>
#include <cstdint>

namespace trackerboy {


class Wav {

public:


    Wav(std::ostream &stream, uint16_t channels, uint32_t samplingRate);

    void begin();

    //
    // Finish writing the header. This should be called after all samples
    // have been written to the file
    //
    void finish();

    void write(float buf[], size_t nsamples);

private:

    std::ostream &mStream;
    size_t mSampleCount;

    uint16_t mChannels;
    uint32_t mSamplingRate;

};



}
