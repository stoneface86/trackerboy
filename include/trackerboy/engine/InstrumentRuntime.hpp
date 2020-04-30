
#pragma once

#include <cstdint>
#include <vector>

#include "trackerboy/Table.hpp"
#include "trackerboy/synth/Synth.hpp"


namespace trackerboy {

class InstrumentRuntimeBase {

public:

    void reset();

    void setProgram(std::vector<uint8_t>* program);

protected:

    InstrumentRuntimeBase();

    std::vector<uint8_t> *mProgram;
    uint8_t mPc;
    int8_t mPitchOffset;
    int8_t mFinePitchOffset;
    bool mRunning;
};


template <ChType ch>
class InstrumentRuntime : public InstrumentRuntimeBase {

public:

    InstrumentRuntime();

    void step(Synth& synth, WaveTable& wtable, uint16_t freq = 0);
};

}
