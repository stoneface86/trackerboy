
#pragma once

#include <cstdint>
#include <vector>

#include "trackerboy/Table.hpp"
#include "trackerboy/instrument/Instruction.hpp"
#include "trackerboy/synth/Synth.hpp"


namespace trackerboy {

class InstrumentRuntime {

public:

    InstrumentRuntime(ChType trackId);

    void reset();

    void setProgram(std::vector<Instruction>* program);

    void step(Synth& synth, WaveTable& wtable, uint8_t rowVol = 0, uint16_t freq = 0);

private:

    std::vector<Instruction> *mProgram;
    uint8_t mFc;
    uint8_t mPc;
    bool mRunning;
    const ChType mTrackId;

};

}
