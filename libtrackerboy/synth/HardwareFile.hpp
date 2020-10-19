/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

#pragma once

#include "synth/Envelope.hpp"
#include "synth/NoiseGen.hpp"
#include "synth/PulseGen.hpp"
#include "synth/Sweep.hpp"
#include "synth/WaveGen.hpp"

namespace trackerboy {

//
// POD struct for the individual hardware components of the synthesizer.
// A number in the field name indicates the channel it belongs to.
//
struct HardwareFile {

    Envelope env1, env2, env4;
    Sweep sweep1;
    PulseGen gen1, gen2;
    WaveGen gen3;
    NoiseGen gen4;

    HardwareFile() noexcept :
        env1(),
        env2(),
        env4(),
        sweep1(gen1),
        gen1(),
        gen2(),
        gen3(),
        gen4()
    {
    }

};



}
